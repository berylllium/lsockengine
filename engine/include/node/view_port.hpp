#pragma once

#include "node/node.hpp"

namespace lise
{

struct ViewPort : public Node
{
    void _enter_tree() override;
    void _init() override;
};

}
