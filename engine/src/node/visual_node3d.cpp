#include "node/visual_node3d.hpp"

namespace lise
{

void VisualNode3D::_notification(int n)
{
    switch (n)
    {
    case NOTIFICATION_DRAW:
    {
        _draw();
    } break;
    default: break;
    }
}

}
