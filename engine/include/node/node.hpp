#pragma once

#include <string>

namespace lise
{

struct NodeTree;

struct Node
{
    std::string node_name;

    // Non-null on root nodes.
    NodeTree* _node_tree;

    // Null on root nodes.
    Node* _parent = nullptr;
    Node* _previous_sibling = nullptr;
    Node* _first_child = nullptr;
    Node* _next_sibling = nullptr;

    void add_child(Node* node);

    // Notifications.
    enum
    {
        // Node
        NOTIFICATION_TREE_CHANGED,
        NOTIFICATION_ENTER_TREE,
        NOTIFICATION_LEFT_TREE,
        NOTIFICATION_INIT,
        NOTIFICATION_PROCESS,
        NOTIFICATION_POST_PROCESS,
        NOTIFICATION_PHYSICS_PROCESS,

        // Node3D
        NOTIFICATION_TRANSFORM_DIRTIED,

        // VisualNode3D
        NOTIFICATION_DRAW
    };

    virtual void _notification(int n);

    void propagate_notification_down(int n, bool traverse_reverse = false);

    void propagate_notification_up(int n);

    virtual void _init() {};

    virtual void _enter_tree() {}

    virtual void _process() {}
    virtual void _post_process() {}
};

}
