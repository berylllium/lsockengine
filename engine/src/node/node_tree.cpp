#include "node/node_tree.hpp"

#include <simple-logger.hpp>

namespace lise
{

void NodeTree::set_root_view_port(ViewPort* new_view_port)
{
    // Unset old view port.
    if (_root_view_port)
    {
        _root_view_port->propagate_notification_down(lise::Node::NOTIFICATION_LEFT_TREE);
        _root_view_port->_node_tree = nullptr;
    }

    _root_view_port = new_view_port;
    new_view_port->_node_tree = this;
    new_view_port->propagate_notification_down(lise::Node::NOTIFICATION_ENTER_TREE);
}

void NodeTree::_tree_changed()
{
    sl::log_debug("Tree has changed.");
}

}
