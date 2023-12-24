#pragma once

#include "node/node3d.hpp"

namespace lise
{

struct VisualNode3D : public Node3D
{
    virtual void _draw() {}

    virtual void _notification(int n) override;
};

}
