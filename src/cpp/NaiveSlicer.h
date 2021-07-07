#pragma once
#include "Layer.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_mesh_slicer.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polyhedron_3.h>

class TriangleGeometry;
namespace Slicer {

class NaiveSlicer
{
public:    
    using K = Layer::K;
    using Mesh = CGAL::Surface_mesh<Layer::Point>;
    using Polyhedron_3 = CGAL::Polyhedron_3<K>;
    using MeshSlicer = CGAL::Polygon_mesh_slicer<Mesh, K>;
    using MeshSlicer2 = CGAL::Polygon_mesh_slicer<Polyhedron_3, K>;

    NaiveSlicer() = default;
	std::vector<Layer> slice(const TriangleGeometry& g, float zStart = 0, float step = 1, uint32_t numSlices = 1);
private:
    //TODO: Clean it up!
	NaiveSlicer::Mesh createMeshSimple(const TriangleGeometry& geom);
	NaiveSlicer::Mesh createMeshReorienting(const TriangleGeometry& geom);
    NaiveSlicer::Mesh createMesh(const TriangleGeometry &geom);
    NaiveSlicer::Polyhedron_3 createMesh2(const TriangleGeometry &geom);
    NaiveSlicer::Polyhedron_3 createMeshFromSTL(const TriangleGeometry &geom, std::string fileName);
};

}
