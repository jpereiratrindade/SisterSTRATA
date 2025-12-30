#pragma once

#include "TimeSlice.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace Core::Domain::FourthDimension {

struct CoherenceIntensityParams {
    int width = 0;
    int height = 0;
    int radius = 2;
    float sigma = 1.0f;
    float weightType = 0.45f;
    float weightStructure = 0.4f;
    float weightEdge = 0.15f;
    int typeCount = 0;
    int noDataCode = -1;
    float offDiagonalSimilarity = 0.25f;
    bool ignoreNoData = true;
    bool ignoreWaterMask = true;
    bool includeComponents = false;
    std::vector<float> typeSimilarity;
};

struct CoherenceIntensityMap {
    int width = 0;
    int height = 0;
    std::vector<float> intensity;
    std::vector<float> sType;
    std::vector<float> sStructure;
    std::vector<float> sEdge;
};

class CoherenceIntensityService {
public:
    static CoherenceIntensityMap compare(
        const TimeSlice& a,
        const TimeSlice& b,
        const CoherenceIntensityParams& params) 
    {
        CoherenceIntensityMap out;

        const auto& stateA = a.getEcologicalCoverState();
        const auto& stateB = b.getEcologicalCoverState();
        const auto& waterA = a.getWaterMask();
        const auto& waterB = b.getWaterMask();

        if (stateA.size() != stateB.size() || stateA.empty()) return out;

        int width = params.width;
        int height = params.height;
        if (!inferGridDimensions(stateA.size(), width, height)) return out;

        out.width = width;
        out.height = height;
        out.intensity.resize(stateA.size(), 0.0f);
        if (params.includeComponents) {
            out.sType.resize(stateA.size(), 0.0f);
            out.sStructure.resize(stateA.size(), 0.0f);
            out.sEdge.resize(stateA.size(), 0.0f);
        }

        int typeCount = params.typeCount;
        if (typeCount <= 0) {
            typeCount = inferTypeCount(stateA, stateB, params.noDataCode);
        }
        const int binCount = typeCount + 1;

        const float sigma = (params.sigma > 0.0f) ? params.sigma : 1.0f;
        const int radius = std::max(0, params.radius);
        const float invTwoSigma2 = 1.0f / (2.0f * sigma * sigma);

        std::vector<OffsetWeight> offsets;
        offsets.reserve(static_cast<size_t>((radius * 2 + 1) * (radius * 2 + 1)));
        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                float d2 = static_cast<float>(dx * dx + dy * dy);
                float w = std::exp(-d2 * invTwoSigma2);
                offsets.push_back({dx, dy, w});
            }
        }

        std::vector<float> histA(binCount, 0.0f);
        std::vector<float> histB(binCount, 0.0f);

        float wType = params.weightType;
        float wStruct = params.weightStructure;
        float wEdge = params.weightEdge;
        normalizeWeights(wType, wStruct, wEdge);

        const bool useWaterMask = params.ignoreWaterMask &&
                                  waterA.size() == stateA.size() &&
                                  waterB.size() == stateA.size();

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const size_t idx = static_cast<size_t>(y * width + x);

                const int codeA = stateA[idx];
                const int codeB = stateB[idx];
                const bool masked = useWaterMask && (waterA[idx] || waterB[idx]);
                const bool skipCell = masked || (params.ignoreNoData && (isNoData(codeA, typeCount, params.noDataCode) ||
                                                                        isNoData(codeB, typeCount, params.noDataCode)));

                float sType = 0.0f;
                float sStruct = 0.0f;
                float sEdge = 0.0f;

                if (!skipCell) {
                    sType = typeSimilarity(codeA, codeB, binCount, params);
                    sStruct = structureSimilarity(stateA, stateB, width, height, x, y, offsets,
                                                  binCount, params, useWaterMask, waterA, waterB, histA, histB);
                    sEdge = edgeSimilarity(stateA, stateB, width, height, x, y, typeCount, params,
                                           useWaterMask, waterA, waterB);
                }

                float intensity = std::clamp(wType * sType + wStruct * sStruct + wEdge * sEdge, 0.0f, 1.0f);
                out.intensity[idx] = intensity;
                if (params.includeComponents) {
                    out.sType[idx] = sType;
                    out.sStructure[idx] = sStruct;
                    out.sEdge[idx] = sEdge;
                }
            }
        }

        return out;
    }

private:
    struct OffsetWeight {
        int dx;
        int dy;
        float weight;
    };

    static bool inferGridDimensions(size_t count, int& width, int& height) {
        if (width > 0 && height > 0) {
            return (static_cast<size_t>(width) * static_cast<size_t>(height) == count);
        }
        int side = static_cast<int>(std::sqrt(static_cast<double>(count)));
        if (side > 0 && static_cast<size_t>(side * side) == count) {
            width = side;
            height = side;
            return true;
        }
        return false;
    }

    static int inferTypeCount(const std::vector<int>& a, const std::vector<int>& b, int noDataCode) {
        int maxCode = -1;
        for (int code : a) {
            if (code != noDataCode && code > maxCode) maxCode = code;
        }
        for (int code : b) {
            if (code != noDataCode && code > maxCode) maxCode = code;
        }
        return maxCode + 1;
    }

    static bool isNoData(int code, int typeCount, int noDataCode) {
        if (code == noDataCode) return true;
        if (code < 0 || code >= typeCount) return true;
        return false;
    }

    static int typeIndex(int code, int typeCount, int noDataCode) {
        if (isNoData(code, typeCount, noDataCode)) return 0;
        return code + 1;
    }

    static float typeSimilarity(int codeA, int codeB, int binCount, const CoherenceIntensityParams& params) {
        if (params.typeSimilarity.size() == static_cast<size_t>(binCount * binCount)) {
            int idxA = typeIndex(codeA, binCount - 1, params.noDataCode);
            int idxB = typeIndex(codeB, binCount - 1, params.noDataCode);
            return params.typeSimilarity[static_cast<size_t>(idxA * binCount + idxB)];
        }
        if (codeA == codeB) return 1.0f;
        return params.offDiagonalSimilarity;
    }

    static void normalizeWeights(float& wType, float& wStruct, float& wEdge) {
        float sum = wType + wStruct + wEdge;
        if (sum <= 0.0f) {
            wType = 1.0f;
            wStruct = 0.0f;
            wEdge = 0.0f;
            return;
        }
        wType /= sum;
        wStruct /= sum;
        wEdge /= sum;
    }

    static float structureSimilarity(
        const std::vector<int>& stateA,
        const std::vector<int>& stateB,
        int width,
        int height,
        int x,
        int y,
        const std::vector<OffsetWeight>& offsets,
        int binCount,
        const CoherenceIntensityParams& params,
        bool useWaterMask,
        const std::vector<bool>& waterA,
        const std::vector<bool>& waterB,
        std::vector<float>& histA,
        std::vector<float>& histB) 
    {
        std::fill(histA.begin(), histA.end(), 0.0f);
        std::fill(histB.begin(), histB.end(), 0.0f);

        float sumA = 0.0f;
        float sumB = 0.0f;
        const int typeCount = binCount - 1;

        for (const auto& off : offsets) {
            int nx = x + off.dx;
            int ny = y + off.dy;
            if (nx < 0 || ny < 0 || nx >= width || ny >= height) continue;

            size_t nidx = static_cast<size_t>(ny * width + nx);

            if (useWaterMask && (waterA[nidx] || waterB[nidx])) continue;

            int codeA = stateA[nidx];
            int codeB = stateB[nidx];

            if (!params.ignoreNoData || !isNoData(codeA, typeCount, params.noDataCode)) {
                int idxA = typeIndex(codeA, typeCount, params.noDataCode);
                histA[idxA] += off.weight;
                sumA += off.weight;
            }
            if (!params.ignoreNoData || !isNoData(codeB, typeCount, params.noDataCode)) {
                int idxB = typeIndex(codeB, typeCount, params.noDataCode);
                histB[idxB] += off.weight;
                sumB += off.weight;
            }
        }

        if (sumA <= 0.0f || sumB <= 0.0f) return 0.0f;

        for (int i = 0; i < binCount; ++i) {
            histA[i] /= sumA;
            histB[i] /= sumB;
        }

        float js = 0.0f;
        constexpr float kLog2 = 0.6931471805599453f;
        for (int i = 0; i < binCount; ++i) {
            float p = histA[i];
            float q = histB[i];
            float m = 0.5f * (p + q);
            if (p > 0.0f) js += 0.5f * p * std::log(p / m);
            if (q > 0.0f) js += 0.5f * q * std::log(q / m);
        }

        float jsNorm = js / kLog2;
        return std::clamp(1.0f - jsNorm, 0.0f, 1.0f);
    }

    static float edgeSimilarity(
        const std::vector<int>& stateA,
        const std::vector<int>& stateB,
        int width,
        int height,
        int x,
        int y,
        int typeCount,
        const CoherenceIntensityParams& params,
        bool useWaterMask,
        const std::vector<bool>& waterA,
        const std::vector<bool>& waterB)
    {
        const size_t idx = static_cast<size_t>(y * width + x);
        if (useWaterMask && (waterA[idx] || waterB[idx])) return 0.0f;

        const int codeA = stateA[idx];
        const int codeB = stateB[idx];
        if (params.ignoreNoData &&
            (isNoData(codeA, typeCount, params.noDataCode) || isNoData(codeB, typeCount, params.noDataCode))) {
            return 0.0f;
        }

        float edgeA = edgeDensity(stateA, width, height, x, y, typeCount, params, useWaterMask, waterA);
        float edgeB = edgeDensity(stateB, width, height, x, y, typeCount, params, useWaterMask, waterB);
        return std::clamp(1.0f - std::abs(edgeA - edgeB), 0.0f, 1.0f);
    }

    static float edgeDensity(
        const std::vector<int>& state,
        int width,
        int height,
        int x,
        int y,
        int typeCount,
        const CoherenceIntensityParams& params,
        bool useWaterMask,
        const std::vector<bool>& waterMask)
    {
        const size_t idx = static_cast<size_t>(y * width + x);
        const int base = state[idx];
        if (useWaterMask && waterMask[idx]) return 0.0f;
        if (params.ignoreNoData && isNoData(base, typeCount, params.noDataCode)) return 0.0f;

        int count = 0;
        int diff = 0;
        const int dx[] = {1, -1, 0, 0};
        const int dy[] = {0, 0, 1, -1};

        for (int k = 0; k < 4; ++k) {
            int nx = x + dx[k];
            int ny = y + dy[k];
            if (nx < 0 || ny < 0 || nx >= width || ny >= height) continue;
            size_t nidx = static_cast<size_t>(ny * width + nx);
            if (useWaterMask && waterMask[nidx]) continue;

            int ncode = state[nidx];
            if (params.ignoreNoData && isNoData(ncode, typeCount, params.noDataCode)) continue;

            count++;
            if (ncode != base) diff++;
        }

        if (count == 0) return 0.0f;
        return static_cast<float>(diff) / static_cast<float>(count);
    }
};

} // namespace Core::Domain::FourthDimension
