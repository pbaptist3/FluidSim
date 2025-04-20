#include "HashContainer.h"

HashContainer::Iterator &HashContainer::Iterator::operator++()
{
    while (c.buckets[i % c.buckets.size()] != -1)
    {
        // quadratic probing
        i += offset++;
        float dx = c.vec()[c.buckets[i % c.buckets.size()]].px - p.px;
        float dy = c.vec()[c.buckets[i % c.buckets.size()]].py - p.py;
        if ((dx*dx+dy*dy)<(radius*radius))
            break;
    }
    return *this;
}

bool HashContainer::Iterator::done()
{
    return c.buckets[i] == -1;
}

int HashContainer::Iterator::idx()
{
    return c.buckets[i];
}

Particle &HashContainer::Iterator::operator*()
{
    return c.vec()[c.buckets[i]];
}

HashContainer::Iterator::Iterator(HashContainer &c, Particle &p, float r) : c(c), p(p), radius(r), offset(1) {
    i = p.hash(3*r);
}

void HashContainer::update(float r)
{
     // *4 is so load factor is 0.25
    buckets = std::vector<int>(particles.size()*4, -1);

    for (int i=0; i<particles.size(); i++)
    {
        const Particle& p = particles[i];
        int idx = p.hash(r*3);
        // quadratic probing
        int offset = 1;
        while (buckets[idx % buckets.size()] != -1) idx += offset++;
        buckets[idx % buckets.size()] = i;
    }
}
