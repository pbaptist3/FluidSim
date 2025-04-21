#ifndef FLUIDSIM_GRAPHCONTAINER_H
#define FLUIDSIM_GRAPHCONTAINER_H

#include "ParticleContainer.h"
#include <unordered_map>
#include <vector>
#include <tuple>
#include <cmath>
#include <functional>

// Custom hash for tuple<int, int>
namespace std {
    template <>
    struct hash<std::tuple<int, int>> {
        size_t operator()(const std::tuple<int, int>& k) const {
            return std::hash<int>()(std::get<0>(k)) ^ (std::hash<int>()(std::get<1>(k)) << 1);
        }
    };
}

class GraphContainer : public ParticleContainer
{
public:
    float cell_size = 0.1f;

    // Maps grid cell (x, y) to list of particle indices
    std::unordered_map<std::tuple<int, int>, std::vector<int>> grid;

    // Rebuilds the grid based on current particle positions
    void build_grid();
};


#endif
