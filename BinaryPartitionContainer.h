#ifndef FLUIDSIM_BINARYPARTITIONCONTAINER_H
#define FLUIDSIM_BINARYPARTITIONCONTAINER_H

#include "ParticleContainer.h"
#include <vector>
#include <queue>

constexpr int MAX_PARTITION = 64;

class BinaryPartitionContainer : public ParticleContainer
{
    struct Node {
        Node* q[4]; // 4 quadrants
        std::vector<int> d; // indices of particles
        std::pair<float, float> center;
        float radius;
        BinaryPartitionContainer& c;

        /// Split the node into four quadrants and appropriately subdivide
        void divide(float r);
        ~Node();
        Node(BinaryPartitionContainer& c) : c(c) {
            q[0] = nullptr;
            q[1] = nullptr;
            q[2] = nullptr;
            q[3] = nullptr;
        }
    };

public:
    class Iterator {
        BinaryPartitionContainer& c;
        Particle& p;
        float r;
        Iterator(BinaryPartitionContainer& c, Particle& p, float r);
        int i;
        std::queue<Node *> to_visit;
    public:
        virtual Iterator& operator++();
        Particle& operator*();
        bool done();
        friend BinaryPartitionContainer;
        int idx();
    };

    /// Recomputes the binary partition
    void update(float r);

    BinaryPartitionContainer() : root(nullptr) {}

private:
    // root node
    Node* root;
};

#endif
