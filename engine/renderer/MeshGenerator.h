#pragma once

#include "Mesh.h"

namespace Vision::MeshGenerator
{

// Three methods to create meshes in engine without needing to import assets.
Mesh* CreatePlaneMesh(float width, float height, float rows, float columns, bool xzCoord = false, bool divideQuads = true);
Mesh* CreateCubeMesh(float size);
Mesh* CreateSphereMesh(float radius, std::size_t numSudivisions = 2);

}