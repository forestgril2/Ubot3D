#include "NaiveSlicer.h"
#include "TriangleGeometry.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_mesh_slicer.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>

#include <QDebug>

#include <CGAL/Surface_mesh.h>

using namespace Slicer;

std::vector<Layer> NaiveSlicer::slice(TriangleGeometry& g)
{
    auto const& mesh = createMesh(g);

    float max = g.boundsMax().z();
    float min = g.boundsMin().z();

    int sliceCount = 5;
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

    std::vector<CGAL::cpp11::array<float, 3> > points;
    std::vector<CGAL::cpp11::array<unsigned int, 3> > triangles;

    for(auto const& v : data.vertices)
    {
        auto arr = CGAL::cpp11::array<float, 3>{v.x(), v.y(), v.z()};
        points.push_back(arr);
    }

    for(size_t i=0; i < data.indices.size(); i+=3)
    {
        auto v1 = data.vertices[data.indices[i]];
        auto v2 = data.vertices[data.indices[i+1]];
        auto v3 = data.vertices[data.indices[i+2]];

        auto arr = CGAL::cpp11::array<unsigned int, 3>{data.indices[i], data.indices[i+1], data.indices[i+2]};
        triangles.push_back(arr);
    }

    Polyhedron_3 poly_Partition;

    CGAL::Polygon_mesh_processing::orient_polygon_soup(points, triangles);
    CGAL::Polygon_mesh_processing::polygon_soup_to_polygon_mesh(points, triangles, poly_Partition);


    CGAL::copy_face_graph(poly_Partition, mesh);


    return mesh;
}

NaiveSlicer::Polyhedron_3 NaiveSlicer::createMesh2(const TriangleGeometry& geom)
{
    Mesh mesh;
    auto data = geom.getData();

    std::vector<CGAL::cpp11::array<float, 3> > points;
    std::vector<CGAL::cpp11::array<unsigned int, 3> > triangles;

    for(auto const& v : data.vertices)
    {
        auto arr = CGAL::cpp11::array<float, 3>{v.x(), v.y(), v.z()};
        points.push_back(arr);
    }

    for(size_t i=0; i < data.indices.size(); i+=3)
    {
        auto v1 = data.vertices[data.indices[i]];
        auto v2 = data.vertices[data.indices[i+1]];
        auto v3 = data.vertices[data.indices[i+2]];

        auto arr = CGAL::cpp11::array<unsigned int, 3>{data.indices[i], data.indices[i+1], data.indices[i+2]};
        triangles.push_back(arr);
    }

    Polyhedron_3 poly_Partition;

    CGAL::Polygon_mesh_processing::orient_polygon_soup(points, triangles);
    CGAL::Polygon_mesh_processing::polygon_soup_to_polygon_mesh(points, triangles, poly_Partition);


    //CGAL::copy_face_graph(poly_Partition, mesh);


    return poly_Partition;
}
