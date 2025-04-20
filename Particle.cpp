#include "Particle.h"

int Particle::hash(float grid_size)
{
    int grid_x = static_cast<int>(px*0xffff) / static_cast<int>(grid_size*0xffff);
    int grid_y = static_cast<int>(py*0xffff) / static_cast<int>(grid_size*0xffff);
    return (grid_y * 0x1c3e4f) ^ grid_x;
}
