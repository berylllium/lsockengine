#include "node/node.hpp"

#include "node/node_tree.hpp"

namespace lise
{

void Node::add_child(Node* new_child)
{
    // Set new child's parent to this node.
    new_child->_parent = this;

    // Set new child as this nodes child.
    if (_first_child == nullptr)
    {
        // No first child.
        _first_child = new_child;
    }
    else
    {
        // Find last child.
        Node* current_child = _first_child;

        while (true)
        {
            if (current_child->_next_sibling == nullptr)
            {
                // Current child is last child.
                current_child->_next_sibling = new_child;
                new_child->_previous_sibling = current_child;

                break;
            }

            current_child = current_child->_next_sibling;
        }
    }
}

void Node::_notification(int n)
{
    switch (n)
    {
    case NOTIFICATION_TREE_CHANGED:
    {
        if (_node_tree) _node_tree->_tree_changed();
    } break;
    case NOTIFICATION_ENTER_TREE:
    {
        _enter_tree();
    } break;
    case NOTIFICATION_INIT:
    {
        _init();
    } break;
    default: break;
    }
}

void Node::propagate_notification_down(int n, bool traverse_reverse)
{
    if (!traverse_reverse)
    {
        _notification(n);
    }

    if (_first_child)
    {
        _first_child->propagate_notification_down(n, traverse_reverse);
    }

    if (_next_sibling)
    {
        _next_sibling->propagate_notification_down(n, traverse_reverse);
    }

    if (traverse_reverse)
    {
        _notification(n);
    }
}

void Node::propagate_notification_up(int n)
{
    _notification(n);

    if (_parent)
    {
        _parent->propagate_notification_up(n);
    }
}

}
