#include "GraphContainer.h"

void GraphContainer::build_grid()
{
    grid.clear();

    for (int i = 0; i < particles.size(); ++i) {
        const Particle& p = particles[i];

        // Convert world position (px, py) to discrete grid cell coordinates
        int cell_x = static_cast<int>(std::floor(p.px / cell_size));
        int cell_y = static_cast<int>(std::floor(p.py / cell_size));

        std::tuple<int, int> key = std::make_tuple(cell_x, cell_y);

        // Store the index of the particle in the appropriate grid cell
        grid[key].push_back(i);
    }
}
