#pragma once

#include <node/view_port.hpp>

namespace lise
{

struct NodeTree
{
    ViewPort* _root_view_port = nullptr;

    void set_root_view_port(ViewPort* new_view_port);

    void _tree_changed();
};

}
