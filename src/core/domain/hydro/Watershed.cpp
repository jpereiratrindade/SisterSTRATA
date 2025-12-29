#include "core/domain/hydro/Watershed.hpp"
#include <queue>
#include <algorithm>
#include <iostream>

namespace Core::Domain::Hydro {

std::vector<uint8_t> Watershed::delineate(HydroGrid& grid, int startX, int startY, int basinID) {
    int w = grid.width;
    int h = grid.height;
    size_t size = static_cast<size_t>(w * h);

    std::vector<uint8_t> mask(size, 0);
    if (startX < 0 || startX >= w || startY < 0 || startY >= h) {
        return mask;
    }

    std::queue<int> q;
    int startIdx = startY * w + startX;
    q.push(startIdx);
    mask[startIdx] = 255;

    if (basinID > 0) {
        if (grid.watershedMap.empty()) {
            grid.watershedMap.assign(size, 0);
        }
        grid.watershedMap[startIdx] = basinID;
    }

    const int dx[] = {0, 1, 1, 1, 0, -1, -1, -1};
    const int dy[] = {-1, -1, 0, 1, 1, 1, 0, -1};

    while (!q.empty()) {
        int idx = q.front();
        q.pop();

        int cx = idx % w;
        int cy = idx / w;

        // Check neighbors that flow INTO current cell
        for (int i = 0; i < 8; ++i) {
            int nx = cx + dx[i];
            int ny = cy + dy[i];
            if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;

            int nIdx = ny * w + nx;
            int receiver = grid.receiverIndex.empty() ? -1 : grid.receiverIndex[nIdx];

            if (receiver == idx && mask[nIdx] == 0) {
                mask[nIdx] = 255;
                if (basinID > 0) grid.watershedMap[nIdx] = basinID;
                q.push(nIdx);
            }
        }
    }

    return mask;
}

int Watershed::segmentGlobal(HydroGrid& grid) {
    int w = grid.width;
    int h = grid.height;
    int size = w * h;
    if (size <= 0) return 0;

    if (grid.watershedMap.size() != static_cast<size_t>(size)) {
        grid.watershedMap.assign(size, 0);
    } else {
        std::fill(grid.watershedMap.begin(), grid.watershedMap.end(), 0);
    }

    int basinCounter = 1;
    std::queue<int> q;

    for (int i = 0; i < size; ++i) {
        int receiver = grid.receiverIndex.empty() ? -1 : grid.receiverIndex[i];
        if (receiver == -1) {
            grid.watershedMap[i] = basinCounter;
            q.push(i);
            basinCounter++;
        }
    }

    // Build upstream adjacency
    std::vector<std::vector<int>> upstream(size);
    upstream.reserve(size);
    for (int i = 0; i < size; ++i) {
        int rec = grid.receiverIndex.empty() ? -1 : grid.receiverIndex[i];
        if (rec >= 0 && rec < size) {
            upstream[rec].push_back(i);
        }
    }

    while (!q.empty()) {
        int curr = q.front();
        q.pop();

        int id = grid.watershedMap[curr];
        for (int up : upstream[curr]) {
            if (grid.watershedMap[up] == 0) {
                grid.watershedMap[up] = id;
                q.push(up);
            }
        }
    }

    std::cout << "[Watershed] Segmented " << (basinCounter - 1) << " basins." << std::endl;
    return basinCounter - 1;
}

} // namespace Core::Domain::Hydro
