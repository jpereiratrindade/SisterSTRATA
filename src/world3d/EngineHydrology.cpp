#include "world3d/Engine.hpp"
#include "core/domain/hydro/DrainageSystem.hpp"
#include "core/domain/hydro/Watershed.hpp"
#include "core/domain/hydro/HydrologyReport.hpp"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace {

float computeGridSpacingXY(const std::vector<World3D::Rendering::Vertex>& vertices, int width, int height) {
    if (vertices.size() < 2 || width <= 0 || height <= 0) return 1.0f;
    float spacing = 0.0f;

    if (width > 1) {
        const auto& a = vertices[0].pos;
        const auto& b = vertices[1].pos;
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        spacing = std::sqrt(dx * dx + dy * dy);
    }
    if (spacing <= 0.0f && height > 1) {
        const auto& a = vertices[0].pos;
        const auto& b = vertices[width].pos;
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        spacing = std::sqrt(dx * dx + dy * dy);
    }

    return (spacing > 0.0f) ? spacing : 1.0f;
}

glm::vec3 hslToRgb(float h, float s, float l) {
    auto hue2rgb = [](float p, float q, float t) {
        if (t < 0.0f) t += 1.0f;
        if (t > 1.0f) t -= 1.0f;
        if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
        if (t < 1.0f / 2.0f) return q;
        if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
        return p;
    };

    float r, g, b;
    if (s == 0.0f) {
        r = g = b = l;
    } else {
        float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
        float p = 2.0f * l - q;
        r = hue2rgb(p, q, h + 1.0f / 3.0f);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1.0f / 3.0f);
    }
    return {r, g, b};
}

glm::vec3 basinColorFromId(int id) {
    if (id <= 0) return {0.6f, 0.6f, 0.6f};
    float hue = std::fmod(id * 0.61803398875f, 1.0f);
    return hslToRgb(hue, 0.55f, 0.55f);
}

} // namespace

namespace World3D {

Engine::DrainageStats Engine::applyDrainageSimulation() {
    DrainageStats stats;
    if (!activeVertices_ || !activeVertexBuffer_) {
        std::cerr << "[Engine] No active mesh to simulate drainage." << std::endl;
        stats.message = "No active mesh to simulate drainage.";
        return stats;
    }

    size_t count = activeVertices_->size();
    int width = static_cast<int>(std::sqrt(count));
    int height = width;

    if (width * height != static_cast<int>(count)) {
        stats.message = "Mesh is not a grid (Vertex count mismatch). Try loading a .csv or Generate Pattern.";
        return stats;
    }

    std::cout << "[Engine] Applying Drainage Simulation (" << width << "x" << height << ")..." << std::endl;

    Core::Domain::Hydro::ElevationGrid terrain;
    terrain.width = width;
    terrain.height = height;
    terrain.z.resize(count);

    for (size_t i = 0; i < count; ++i) {
        terrain.z[i] = (*activeVertices_)[i].pos.z;
    }

    Core::Domain::Hydro::DrainageSystem::process(terrain, lastHydroGrid_);

    long totalAcc = 0;
    stats.maxAccumulation = 0;
    stats.riverCells = 0;

    for (auto acc : lastHydroGrid_.flowAccumulationCells) {
        if (acc > stats.maxAccumulation) stats.maxAccumulation = acc;
        totalAcc += acc;
        if (acc > 50) stats.riverCells++;
    }

    stats.meanAccumulation = (count > 0) ? static_cast<float>(totalAcc) / count : 0.0f;
    stats.message = "";
    lastDrainageStats_ = stats;

    std::cout << "[Engine] Drainage Analysis Complete." << std::endl;
    return stats;
}

bool Engine::setDrainageVisualization(bool showDrainage, bool showWatersheds, bool showBasinOutlines, float intensity) {
    if (!activeVertices_ || !activeVertexBuffer_) {
        std::cerr << "[Engine] No active mesh to visualize drainage." << std::endl;
        return false;
    }

    size_t count = activeVertices_->size();
    int width = static_cast<int>(std::sqrt(count));
    int height = width;
    if (width * height != static_cast<int>(count)) {
        std::cerr << "[Engine] Mesh is not a grid (Vertex count mismatch)." << std::endl;
        return false;
    }

    auto& verts = *activeVertices_;

    if (!showDrainage && !showWatersheds) {
        if (baseColorsValid_ && baseColors_.size() == count) {
            for (size_t i = 0; i < count; ++i) {
                verts[i].color = baseColors_[i];
            }
        }
        hydroVisMode_ = HydroVisMode::None;
        baseColorsValid_ = false;
    } else {
        if (!baseColorsValid_ || baseColors_.size() != count || hydroVisMode_ == HydroVisMode::None) {
            baseColors_.resize(count);
            for (size_t i = 0; i < count; ++i) {
                baseColors_[i] = verts[i].color;
            }
            baseColorsValid_ = true;
        }

        if (lastHydroGrid_.flowAccumulationCells.size() != count) {
            DrainageStats stats = applyDrainageSimulation();
            if (!stats.message.empty()) return false;
        }

        if (showWatersheds) {
            if (lastHydroGrid_.watershedMap.size() != count) {
                lastHydroGrid_.watershedMap.assign(count, 0);
            }

            bool hasBasins = false;
            for (int id : lastHydroGrid_.watershedMap) {
                if (id > 0) { hasBasins = true; break; }
            }
            if (!hasBasins) {
                Core::Domain::Hydro::Watershed::segmentGlobal(lastHydroGrid_);
            }

            const int dx[] = {0, 1, 1, 1, 0, -1, -1, -1};
            const int dy[] = {-1, -1, 0, 1, 1, 1, 0, -1};

            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    int idx = y * width + x;
                    int id = lastHydroGrid_.watershedMap[idx];
                    glm::vec3 color = (id > 0) ? basinColorFromId(id) : baseColors_[idx];

                    if (showBasinOutlines && id > 0) {
                        bool edge = false;
                        for (int k = 0; k < 8; ++k) {
                            int nx = x + dx[k];
                            int ny = y + dy[k];
                            if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
                            int nIdx = ny * width + nx;
                            int nId = lastHydroGrid_.watershedMap[nIdx];
                            if (nId != id) {
                                edge = true;
                                break;
                            }
                        }
                        if (edge) {
                            color = glm::vec3(0.05f, 0.05f, 0.05f);
                        }
                    }

                    verts[idx].color = color;
                }
            }
            hydroVisMode_ = HydroVisMode::Watershed;
        } else {
            float maxLogAcc = 1.0f;
            int maxAccum = 0;
            for (auto acc : lastHydroGrid_.flowAccumulationCells) {
                if (acc > maxAccum) maxAccum = acc;
                if (acc > 0) {
                    float l = std::log(static_cast<float>(acc));
                    if (l > maxLogAcc) maxLogAcc = l;
                }
            }

            float intensityClamped = std::clamp(intensity, 0.05f, 1.0f);
            int threshold = std::max(1, static_cast<int>(maxAccum * intensityClamped * 0.02f));
            float scale = 0.5f + intensityClamped * 1.5f;

            for (size_t i = 0; i < count; ++i) {
                int acc = lastHydroGrid_.flowAccumulationCells[i];
                glm::vec3 base = baseColors_[i];
                if (acc >= threshold) {
                    float t = std::log(static_cast<float>(acc)) / maxLogAcc;
                    t = std::clamp(t * scale, 0.0f, 1.0f);
                    glm::vec3 river = glm::mix(glm::vec3(0.0f, 0.0f, 0.5f), glm::vec3(0.0f, 0.8f, 1.0f), t);
                    verts[i].color = glm::mix(base, river, t);
                } else {
                    verts[i].color = base;
                }
            }
            hydroVisMode_ = HydroVisMode::Drainage;
        }
    }

    if (activeTopology_ == vk::PrimitiveTopology::ePointList) {
        if (renderer_) renderer_->setPointSize(4.0f);
    }

    vk::DeviceSize size = sizeof(Rendering::Vertex) * verts.size();
    Rendering::Buffer staging(*context_, size, vk::BufferUsageFlagBits::eTransferSrc,
                              vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    staging.copyTo(verts.data(), size);
    renderer_->copyBuffer(staging.getHandle(), activeVertexBuffer_->getHandle(), size);

    return true;
}

::Core::Domain::Hydro::HydrologyStats Engine::getHydrologyStats(float streamThreshold) {
    ::Core::Domain::Hydro::HydrologyStats stats;
    if (!activeVertices_) return stats;

    size_t count = activeVertices_->size();
    int width = static_cast<int>(std::sqrt(count));
    int height = width;
    if (width * height != static_cast<int>(count)) return stats;

    if (lastHydroGrid_.flowAccumulationCells.size() != count) {
        DrainageStats dStats = applyDrainageSimulation();
        if (!dStats.message.empty()) return stats;
    }

    bool hasBasins = false;
    if (lastHydroGrid_.watershedMap.size() == count) {
        for (int id : lastHydroGrid_.watershedMap) {
            if (id > 0) { hasBasins = true; break; }
        }
    }
    if (!hasBasins) {
        Core::Domain::Hydro::Watershed::segmentGlobal(lastHydroGrid_);
    }

    Core::Domain::Hydro::ElevationGrid terrain;
    terrain.width = width;
    terrain.height = height;
    terrain.z.resize(count);
    for (size_t i = 0; i < count; ++i) {
        terrain.z[i] = (*activeVertices_)[i].pos.z;
    }

    float spacing = computeGridSpacingXY(*activeVertices_, width, height);
    stats = Core::Domain::Hydro::HydrologyReport::analyze(terrain, lastHydroGrid_, spacing, streamThreshold);
    lastHydrologyStats_ = stats;
    return stats;
}

std::pair<bool, std::string> Engine::generateHydrologyReport(const std::string& filepath, float streamThreshold) {
    if (!activeVertices_) return {false, "No active vertices."};

    size_t count = activeVertices_->size();
    int width = static_cast<int>(std::sqrt(count));
    int height = width;
    if (width * height != static_cast<int>(count)) return {false, "Vertex count mismatch (not a grid)."};

    if (lastHydroGrid_.flowAccumulationCells.size() != count) {
        DrainageStats dStats = applyDrainageSimulation();
        if (!dStats.message.empty()) return {false, dStats.message};
    }

    Core::Domain::Hydro::Watershed::segmentGlobal(lastHydroGrid_);

    Core::Domain::Hydro::ElevationGrid terrain;
    terrain.width = width;
    terrain.height = height;
    terrain.z.resize(count);
    for (size_t i = 0; i < count; ++i) {
        terrain.z[i] = (*activeVertices_)[i].pos.z;
    }

    float spacing = computeGridSpacingXY(*activeVertices_, width, height);

    std::filesystem::path p(filepath);
    if (p.has_parent_path() && !std::filesystem::exists(p.parent_path())) {
         return {false, "Directory does not exist."};
    }

    std::string tempPath = filepath + ".tmp";
    bool success = Core::Domain::Hydro::HydrologyReport::generateToFile(terrain, lastHydroGrid_, spacing, tempPath, streamThreshold);

    if (success) {
        try {
            std::filesystem::rename(tempPath, filepath);
            return {true, "Report saved successfully."};
        } catch (const std::filesystem::filesystem_error& e) {
            return {false, std::string("Failed to rename temp file: ") + e.what()};
        }
    } else {
        return {false, "Failed to generate report content."};
    }
}

std::pair<bool, std::string> Engine::exportBasinBoundariesCsv(const std::string& filepath) {
    if (!activeVertices_) return {false, "No active vertices."};

    size_t count = activeVertices_->size();
    int width = static_cast<int>(std::sqrt(count));
    int height = width;
    if (width * height != static_cast<int>(count)) return {false, "Vertex count mismatch."};

    if (lastHydroGrid_.flowAccumulationCells.size() != count) {
        DrainageStats dStats = applyDrainageSimulation();
        if (!dStats.message.empty()) return {false, dStats.message};
    }

    bool hasBasins = false;
    if (lastHydroGrid_.watershedMap.size() == count) {
        for (int id : lastHydroGrid_.watershedMap) {
            if (id > 0) { hasBasins = true; break; }
        }
    }
    if (!hasBasins) {
        Core::Domain::Hydro::Watershed::segmentGlobal(lastHydroGrid_);
    }

    std::vector<uint8_t> boundaryMask = Core::Domain::Hydro::Watershed::computeBoundaryMask(lastHydroGrid_);
    if (boundaryMask.size() != count) return {false, "Boundary mask computation failed."};

    std::filesystem::path p(filepath);
    if (p.has_parent_path() && !std::filesystem::exists(p.parent_path())) {
         return {false, "Target directory does not exist."};
    }

    std::string tempPath = filepath + ".tmp";
    std::ofstream out(tempPath);
    if (!out.is_open()) return {false, "Could not create temporary file."};

    out << "line_id,seq,x,y,z,r,g,b,basin_id\n";
    int lineId = 1;
    auto idx = [width](int x, int y) { return y * width + x; };

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int i = idx(x, y);
            if (boundaryMask[i] == 0) continue;

            const auto& v = (*activeVertices_)[i];
            int basinId = (i < static_cast<int>(lastHydroGrid_.watershedMap.size())) ? lastHydroGrid_.watershedMap[i] : 0;
            glm::vec3 color = basinColorFromId(basinId);

            auto emitSegment = [&](int x2, int y2) {
                int j = idx(x2, y2);
                const auto& v2 = (*activeVertices_)[j];
                float z1 = v.pos.z + 0.02f;
                float z2 = v2.pos.z + 0.02f;
                out << lineId << ",0," << v.pos.x << "," << v.pos.y << "," << z1 << ","
                    << color.r << "," << color.g << "," << color.b << "," << basinId << "\n";
                out << lineId << ",1," << v2.pos.x << "," << v2.pos.y << "," << z2 << ","
                    << color.r << "," << color.g << "," << color.b << "," << basinId << "\n";
                ++lineId;
            };

            if (x + 1 < width) {
                int j = idx(x + 1, y);
                if (boundaryMask[j] != 0) {
                    emitSegment(x + 1, y);
                }
            }
            if (y + 1 < height) {
                int j = idx(x, y + 1);
                if (boundaryMask[j] != 0) {
                    emitSegment(x, y + 1);
                }
            }
        }
    }
    out.close();

    try {
        std::filesystem::rename(tempPath, filepath);
        return {true, "Basin boundaries saved successfully."};
    } catch (const std::filesystem::filesystem_error& e) {
        return {false, std::string("Failed to rename temp file: ") + e.what()};
    }
}

} // namespace World3D
