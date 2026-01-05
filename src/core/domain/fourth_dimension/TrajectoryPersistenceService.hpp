#pragma once

#include "TimeSlice.hpp"
#include <fstream>
#include <vector>
#include <cstdint>

namespace Core::Domain::FourthDimension {

/**
 * @brief Domain Service responsible for binary serialization of TimeSlices.
 * Enables LOD Temporal (Persistence) to save RAM.
 */
class TrajectoryPersistenceService {
public:
    /**
     * @brief Saves the slice data to disk and unloads it from RAM.
     * @param slice The slice to persist.
     * @param directory The directory where to save the binary.
     * @return true if successful.
     */
    static bool saveToDisk(TimeSlice& slice, const std::string& directory) {
        std::string filename = directory + "/timeslice_" + std::to_string(slice.getOrdinalIndex()) + ".bin";
        std::ofstream oss(filename, std::ios::binary);
        if (!oss) return false;

        const auto& cover = slice.getEcologicalCoverState();
        const auto& water = slice.getWaterMask();

        uint32_t coverSize = static_cast<uint32_t>(cover.size());
        uint32_t waterSize = static_cast<uint32_t>(water.size());

        // Header: Sizes
        oss.write(reinterpret_cast<const char*>(&coverSize), sizeof(uint32_t));
        oss.write(reinterpret_cast<const char*>(&waterSize), sizeof(uint32_t));

        // Data: Cover
        if (coverSize > 0) {
            oss.write(reinterpret_cast<const char*>(cover.data()), coverSize * sizeof(int));
        }
        
        // Data: Water (Vector<bool> proxy conversion)
        if (waterSize > 0) {
            std::vector<uint8_t> waterInternal(water.begin(), water.end());
            oss.write(reinterpret_cast<const char*>(waterInternal.data()), waterSize * sizeof(uint8_t));
        }

        slice.setDiskPath(filename);
        slice.unload();
        return true;
    }

    /**
     * @brief Reloads the slice data from disk.
     * @param slice The proxy slice to load.
     * @return true if successful.
     */
    static bool loadFromDisk(TimeSlice& slice) {
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
};

} // namespace Core::Domain::FourthDimension
