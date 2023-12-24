#pragma once

#include "node/visual_node3d.hpp"
#include "renderer/resource/mesh.hpp"

namespace lise
{
    
struct MeshRenderer3D : public VisualNode3D
{
    Mesh _mesh;

    virtual void _draw() override;
};

}
