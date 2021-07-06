#include "NaiveSlicer.h"
#include "TriangleGeometry.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_mesh_slicer.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Surface_mesh.h>

#include <CGAL/Polygon_mesh_processing/orient_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/polygon_soup_to_polygon_mesh.h>
#include <CGAL/Polygon_mesh_processing/orientation.h>

using namespace Slicer;

typedef CGAL::Simple_cartesian<double>     Kernel;
typedef CGAL::Polyhedron_3<Kernel>         Polyhedron;

std::vector<Layer> NaiveSlicer::slice(TriangleGeometry& g, float zStart, float step, uint32_t numSlices)
{
	auto const mesh = createMeshReorienting(g);

	const float zMax = g.boundsMax().z();
	const float zMin = g.boundsMin().z();

    MeshSlicer cgalSlicer(mesh);

    auto makeSlice = [](auto& slicer, auto intersectionPlane){
        Layer::Polylines polylines;
        slicer(intersectionPlane, std::back_inserter(polylines));
        return polylines;
    };

    std::vector<Layer> layers;
	for(uint32_t i=0; i<numSlices; ++i)
    {
		const float z = zStart + float(i)*step;
		if (z < zMin || z > zMax)
			continue;

		layers.emplace_back(Layer(makeSlice(cgalSlicer, K::Plane_3(0, 0, -1, z))));
    }

    return layers;
}

NaiveSlicer::Mesh NaiveSlicer::createMeshSimple(const TriangleGeometry& geom)
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

NaiveSlicer::Mesh NaiveSlicer::createMeshReorienting(const TriangleGeometry& geom)
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

	Polyhedron poly_Partition;

	CGAL::Polygon_mesh_processing::orient_polygon_soup(points, triangles);
	CGAL::Polygon_mesh_processing::polygon_soup_to_polygon_mesh(points, triangles, poly_Partition);

	CGAL::copy_face_graph(poly_Partition, mesh);

	return mesh;
}
