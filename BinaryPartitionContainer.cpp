#include "BinaryPartitionContainer.h"
#include <cmath>

BinaryPartitionContainer::Iterator::Iterator(BinaryPartitionContainer &c, Particle &p, float r) : c(c), p(p), i(0), r(r)
{
    to_visit.push(c.root);
    for (;;)
    {
        i = 0;
        if (!to_visit.front()->q[0] && !to_visit.front()->q[1] && !to_visit.front()->q[2] && !to_visit.front()->q[3])
        {
            while (i < to_visit.front()->d.size())
            {
                float dx = c.vec()[i].px - p.px;
                float dy = c.vec()[i].py - p.py;
                if ((dx*dx+dy*dy)<(r*r))
                    break;
                ++i;
            }
            to_visit.pop();
        }

        for (auto& quad : to_visit.front()->q)
        {
            if (abs(quad->center.first - p.px) < quad->radius + r && abs(quad->center.second - p.py) < quad->radius + r)
                to_visit.push(quad);
        }
        to_visit.pop();
    }
}

int BinaryPartitionContainer::Iterator::idx()
{
    return to_visit.front()->d[i];
}

Particle& BinaryPartitionContainer::Iterator::operator*()
{
    return c.vec()[idx()];
}

bool BinaryPartitionContainer::Iterator::done()
{
    return to_visit.empty();
}

BinaryPartitionContainer::Iterator& BinaryPartitionContainer::Iterator::operator++()
{
    if (++i < to_visit.front()->d.size())
        return *this;

    i = 0;
    to_visit.pop();
    for (;;)
    {
        if (!to_visit.front()->q[0] && !to_visit.front()->q[1] && !to_visit.front()->q[2] && !to_visit.front()->q[3])
        {
            while (i < to_visit.front()->d.size())
            {
                float dx = c.vec()[i].px - p.px;
                float dy = c.vec()[i].py - p.py;
                if ((dx*dx+dy*dy)<(r*r))
                    break;
                ++i;
            }
            to_visit.pop();
        }

        for (auto& quad : to_visit.front()->q)
        {
            if (abs(quad->center.first - p.px) < quad->radius + r && abs(quad->center.second - p.py) < quad->radius + r)
                to_visit.push(quad);
        }
        to_visit.pop();
    }

    return *this;
}

void BinaryPartitionContainer::update(float r)
{
    // remove partitions
    delete root;

    // make new root
    root = new Node(*this);
    root->radius = 1.0;
    root->center = {0.0, 0.0};
    for (int i=0; i<particles.size(); i++)
        root->d.push_back(i);

    // repartition
    root->divide(r);
}

void BinaryPartitionContainer::Node::divide(float r)
{
    Node* q1 = new Node(c);
    q1->center = {center.first + radius/2, center.first + radius/2};
    Node* q2 = new Node(c);
    q2->center = {center.first - radius/2, center.first + radius/2};
    Node* q3 = new Node(c);
    q3->center = {center.first - radius/2, center.first - radius/2};
    Node* q4 = new Node(c);
    q4->center = {center.first + radius/2, center.first - radius/2};
    float new_r = radius/2;
    q1->radius = new_r;
    q2->radius = new_r;
    q3->radius = new_r;
    q4->radius = new_r;

    q[0] = q1;
    q[1] = q2;
    q[2] = q3;
    q[3] = q4;
    for (const auto& idx : d)
    {
        const auto& part = c.vec()[idx];
        if (part.px > center.first && part.py > center.second)
            q1->d.push_back(idx);
        if (part.px < center.first && part.py > center.second)
            q2->d.push_back(idx);
        if (part.px < center.first && part.py < center.second)
            q3->d.push_back(idx);
        if (part.px > center.first && part.py < center.second)
            q4->d.push_back(idx);
    }

    for (auto& quad : q)
    {
        if (quad->radius > r && quad->d.size() > MAX_PARTITION)
            quad->divide(r);
    }
}

BinaryPartitionContainer::Node::~Node()
{
    for (Node* quad : q)
        delete quad;
}