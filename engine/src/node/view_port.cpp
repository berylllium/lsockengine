#include <simple-logger.hpp>
#include "node/view_port.hpp"

namespace lise
{

void ViewPort::_enter_tree()
{
    sl::log_debug("ViewPort {} entered tree.", node_name);
}

void ViewPort::_init()
{
    sl::log_debug("ViewPort {} is initializing.", node_name);
}

}
