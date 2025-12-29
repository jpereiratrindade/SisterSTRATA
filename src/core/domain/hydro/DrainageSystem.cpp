#include "core/domain/hydro/DrainageSystem.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <iostream>

namespace Core::Domain::Hydro {

// Helper: Direction offsets (dx, dy) and distance.
// 0=E, 1=SE, 2=S, 3=SW, 4=W, 5=NW, 6=N, 7=NE
static const int dx8[8] = { 1,  1,  0, -1, -1, -1,  0,  1 };
static const int dy8[8] = { 0,  1,  1,  1,  0, -1, -1, -1 };
static const float dist8[8] = { 1.0f, 1.41421356f, 1.0f, 1.41421356f, 1.0f, 1.41421356f, 1.0f, 1.41421356f };
static const FlowDir dirMap[8] = { 
    FlowDir::East, FlowDir::SouthEast, FlowDir::South, FlowDir::SouthWest, 
    FlowDir::West, FlowDir::NorthWest, FlowDir::North, FlowDir::NorthEast 
};

void DrainageSystem::process(const ElevationGrid& terrain, HydroGrid& grid, SinkHandling sinkMethod) {
    if (terrain.width <= 0 || terrain.height <= 0) {
        std::cerr << "[DrainageSystem] Invalid terrain dimensions." << std::endl;
        return;
    }

    grid.width = terrain.width;
    grid.height = terrain.height;
    grid.flowDirection.resize(grid.width * grid.height);
    grid.receiverIndex.resize(grid.width * grid.height);
    grid.flowAccumulationCells.resize(grid.width * grid.height);
    grid.slope.resize(grid.width * grid.height);
    grid.watershedMap.resize(grid.width * grid.height);

    calculateFlowDirection(terrain, grid);
    calculateFlowAccumulation(grid);
}

void DrainageSystem::calculateFlowDirection(const ElevationGrid& terrain, HydroGrid& grid) {
    int w = grid.width;
    int h = grid.height;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            float currentZ = terrain.get(x, y);
            
            float maxSlope = 0.0f;
            FlowDir bestDir = FlowDir::Sink;
            int bestReceiver = -1;

            // Check 8 neighbors
            for (int d = 0; d < 8; ++d) {
                int nx = x + dx8[d];
                int ny = y + dy8[d];

                if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                    float neighborZ = terrain.get(nx, ny);
                    float drop = currentZ - neighborZ;
                    
                    if (drop > 0.0f) {
                        float slope = drop / dist8[d];
                        if (slope > maxSlope) {
                            maxSlope = slope;
                            bestDir = dirMap[d];
                            bestReceiver = ny * w + nx;
                        }
                    }
                }
            }
            grid.flowDirection[idx] = bestDir;
            grid.receiverIndex[idx] = bestReceiver;
            grid.slope[idx] = maxSlope;
        }
    }
}

void DrainageSystem::calculateFlowAccumulation(HydroGrid& grid) {
    int size = grid.width * grid.height;
    std::fill(grid.flowAccumulationCells.begin(), grid.flowAccumulationCells.end(), 0);

    std::vector<int> inDegree(size, 0);
    
    // 1. Calculate In-Degree (number of cells flowing INTO this cell)
    for (int i = 0; i < size; ++i) {
        int receiver = grid.receiverIndex[i];
        if (receiver >= 0 && receiver < size) {
            inDegree[receiver]++;
        }
    }

    // 2. Initialize Queue with cells that have 0 inflow
    // Base accumulation = 1 (Rainfall on self)
    std::fill(grid.flowAccumulationCells.begin(), grid.flowAccumulationCells.end(), 1);

    std::vector<int> queue;
    queue.reserve(size / 10); 

    for (int i = 0; i < size; ++i) {
        if (inDegree[i] == 0) {
            queue.push_back(i);
        }
    }

    // 3. Process Queue
    int head = 0;
    while(head < queue.size()) {
        int u = queue[head++];

        // Pass flow to neighbor
        int v = grid.receiverIndex[u];
        if (v >= 0 && v < size) {
            // Accumulate u's flow into v
            grid.flowAccumulationCells[v] += grid.flowAccumulationCells[u];

            // Remove dependency
            inDegree[v]--;
            if (inDegree[v] == 0) {
                queue.push_back(v);
            }
        }
    }
}

} // namespace Core::Domain::Hydro
