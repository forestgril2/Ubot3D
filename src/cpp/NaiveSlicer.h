#pragma once
#include "Layer.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_mesh_slicer.h>
#include <CGAL/Surface_mesh.h>

class TriangleGeometry;
namespace Slicer {

class NaiveSlicer
{
public:    
    using K = Layer::K;
    using Mesh = CGAL::Surface_mesh<Layer::Point>;
    using MeshSlicer = CGAL::Polygon_mesh_slicer<Mesh, K>;

    NaiveSlicer() = default;
    std::vector<Layer> slice(TriangleGeometry& g);
private:
    NaiveSlicer::Mesh createMesh(const TriangleGeometry &geom);
};

}
