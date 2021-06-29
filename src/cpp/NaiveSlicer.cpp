#include "NaiveSlicer.h"
#include "TriangleGeometry.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_mesh_slicer.h>
#include <CGAL/Surface_mesh.h>

#include <CGAL/Surface_mesh.h>

using namespace Slicer;

std::vector<Layer> NaiveSlicer::slice(TriangleGeometry& g)
{
    auto const& mesh = createMesh(g);

    float max = g.boundsMax().z();
    float min = g.boundsMin().z();

    int sliceCount = 100;
    float step = std::abs(max-min) / sliceCount;
    float z = max;

    MeshSlicer cgalSlicer(mesh);

    auto makeSlice = [](auto& slicer, auto intersectionPlane){
        Layer::Polylines polylines;
        slicer(intersectionPlane, std::back_inserter(polylines));
        return polylines;
    };

    std::vector<Layer> layers;
    for(int i=0;i<sliceCount;i++)
    {
        z -= step;
        auto const& polylines = makeSlice(cgalSlicer, K::Plane_3(0,0,-1,z));
        layers.push_back(Layer(polylines));
    }

    return layers;
}

NaiveSlicer::Mesh NaiveSlicer::createMesh(const TriangleGeometry& geom)
{
    Mesh mesh;
    auto data = geom.getData();


    for(size_t i=0; i < data.indices.size(); i+=3)
    {
        auto v1 = data.vertices[data.indices[i]];
        auto v2 = data.vertices[data.indices[i+1]];
        auto v3 = data.vertices[data.indices[i+2]];

        mesh.add_face(
                    mesh.add_vertex(Layer::Point(v1.x(), v1.y(), v1.z())),
                    mesh.add_vertex(Layer::Point(v2.x(), v2.y(), v2.z())),
                    mesh.add_vertex(Layer::Point(v3.x(), v3.y(), v3.z())));
    }

    return mesh;
}
