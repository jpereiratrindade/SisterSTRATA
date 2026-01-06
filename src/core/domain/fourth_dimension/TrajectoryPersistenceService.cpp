#include "TrajectoryPersistenceService.hpp"
#include "Trajectory.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdint>

namespace Core::Domain::FourthDimension {

bool TrajectoryPersistenceService::saveToDisk(TimeSlice& slice, const std::string& directory) {
    std::string filename = directory + "/timeslice_" + std::to_string(slice.getOrdinalIndex()) + ".bin";
    std::ofstream oss(filename, std::ios::binary);
    if (!oss) return false;

    const auto& cover = slice.getEcologicalCoverState();
    const auto& water = slice.getWaterMask();

    uint32_t coverSize = static_cast<uint32_t>(cover.size());
    uint32_t waterSize = static_cast<uint32_t>(water.size());

    oss.write(reinterpret_cast<const char*>(&coverSize), sizeof(uint32_t));
    oss.write(reinterpret_cast<const char*>(&waterSize), sizeof(uint32_t));

    if (coverSize > 0) {
        oss.write(reinterpret_cast<const char*>(cover.data()), coverSize * sizeof(int));
    }
    
    if (waterSize > 0) {
        std::vector<uint8_t> waterInternal(water.begin(), water.end());
        oss.write(reinterpret_cast<const char*>(waterInternal.data()), waterSize * sizeof(uint8_t));
    }

    slice.setDiskPath(filename);
    slice.unload();
    return true;
}

bool TrajectoryPersistenceService::loadFromDisk(TimeSlice& slice) {
    if (!slice.isProxy() || slice.getDiskPath().empty()) return true;

    std::ifstream iss(slice.getDiskPath(), std::ios::binary);
    if (!iss) return false;

    uint32_t coverSize = 0;
    uint32_t waterSize = 0;
    
    iss.read(reinterpret_cast<char*>(&coverSize), sizeof(uint32_t));
    iss.read(reinterpret_cast<char*>(&waterSize), sizeof(uint32_t));

    std::vector<int> cover(coverSize);
    if (coverSize > 0) {
        iss.read(reinterpret_cast<char*>(cover.data()), coverSize * sizeof(int));
    }

    std::vector<bool> water(waterSize);
    if (waterSize > 0) {
        std::vector<uint8_t> waterInternal(waterSize);
        iss.read(reinterpret_cast<char*>(waterInternal.data()), waterSize * sizeof(uint8_t));
        water.assign(waterInternal.begin(), waterInternal.end());
    }

    slice.load(cover, water);
    return true;
}

bool TrajectoryPersistenceService::saveTrajectory(Trajectory& trajectory, const std::string& directory, const std::string& manifestName) {
    std::string manifestPath = directory + "/" + manifestName;
    std::ofstream oss(manifestPath);
    if (!oss) return false;

    for (auto& slice : trajectory.getTimeSlices()) {
        if (!slice.isProxy()) {
            saveToDisk(slice, directory);
        }
        
        oss << "SLICE " << slice.getId() << "\n";
        oss << "  ORDINAL " << slice.getOrdinalIndex() << "\n";
        oss << "  METADATA " << slice.getMetadata() << "\n";
        oss << "  TYPE " << static_cast<int>(slice.getClassificationType()) << "\n";
        oss << "  PATH " << slice.getDiskPath() << "\n";
        oss << "SLICE_END\n";
    }
    return true;
}

bool TrajectoryPersistenceService::loadTrajectory(Trajectory& trajectory, const std::string& directory, const std::string& manifestName) {
    std::string manifestPath = directory + "/" + manifestName;
    std::ifstream iss(manifestPath);
    if (!iss) return false;

    trajectory.clear();
    std::string line;
    
    int id = 0, ord = 0, typeInt = 0;
    std::string meta, path;

    while (std::getline(iss, line)) {
        std::stringstream ss(line);
        std::string key;
        ss >> key;

        if (key == "SLICE") ss >> id;
        else if (key == "ORDINAL") ss >> ord;
        else if (key == "METADATA") {
            std::getline(ss, meta);
            if (!meta.empty() && meta[0] == ' ') meta.erase(0, 1);
        }
        else if (key == "TYPE") ss >> typeInt;
        else if (key == "PATH") ss >> path;
        else if (key == "SLICE_END") {
            TimeSlice slice(id, ord, {}, {}, meta, static_cast<ClassificationType>(typeInt));
            slice.setDiskPath(path);
            trajectory.addTimeSlice(slice);
        }
    }
    return true;
}

} // namespace Core::Domain::FourthDimension
