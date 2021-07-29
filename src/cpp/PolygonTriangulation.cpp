#include "PolygonTriangulation.h"

#include <iostream>
#include <vector>
#include <list>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Polygon_2.h>

#include <CommonDefs.h>
using Vec3 = Vertex;

struct FaceInfo2
{
  FaceInfo2(){}
  int nesting_level;
  bool in_domain(){
	return nesting_level%2 == 1;
  }
};

typedef CGAL::Exact_predicates_inexact_constructions_kernel       K;
typedef CGAL::Triangulation_vertex_base_2<K>                      Vb;
typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo2,K>    Fbb;
typedef CGAL::Constrained_triangulation_face_base_2<K,Fbb>        Fb;
typedef CGAL::Triangulation_data_structure_2<Vb,Fb>               TDS;
typedef CGAL::Exact_predicates_tag                                Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag>  CDT;
typedef CDT::Point                                                Point;
typedef CGAL::Polygon_2<K>                                        Polygon_2;
typedef CDT::Face_handle                                          Face_handle;

void mark_domains(CDT& ct,
				  Face_handle start,
				  int index,
				  std::list<CDT::Edge>& border )
{
  if(start->info().nesting_level != -1){
	return;
  }
  std::list<Face_handle> queue;
  queue.push_back(start);
  while(! queue.empty()){
	Face_handle fh = queue.front();
	queue.pop_front();
	if(fh->info().nesting_level == -1){
	  fh->info().nesting_level = index;
	  for(int i = 0; i < 3; i++){
		CDT::Edge e(fh,i);
		Face_handle n = fh->neighbor(i);
		if(n->info().nesting_level == -1){
		  if(ct.is_constrained(e)) border.push_back(e);
		  else queue.push_back(n);
		}
	  }
	}
  }
}
//explore set of facets connected with non constrained edges,
//and attribute to each such set a nesting level.
//We start from facets incident to the infinite vertex, with a nesting
//level of 0. Then we recursively consider the non-explored facets incident
//to constrained edges bounding the former set and increase the nesting level by 1.
//Facets in the domain are those with an odd nesting level.
void mark_domains(CDT& cdt)
{
  for(CDT::Face_handle f : cdt.all_face_handles()){
	f->info().nesting_level = -1;
  }
  std::list<CDT::Edge> border;
  mark_domains(cdt, cdt.infinite_face(), 0, border);
  while(! border.empty()){
	CDT::Edge e = border.front();
	border.pop_front();
	Face_handle n = e.first->neighbor(e.second);
	if(n->info().nesting_level == -1){
	  mark_domains(cdt, n, e.first->info().nesting_level+1, border);
	}
  }
}

PolygonTriangulation::PolygonTriangulation(const std::vector<Vec3>& vertices,
										   const std::list<std::vector<uint32_t>>& boundaryNodeRings,
										   const float zLevel,
										   const Vec3& meshNormal)
{
	CDT cdt;

	for(const auto& nodeRing : boundaryNodeRings)
	{
		Polygon_2 polygon1;
		for (uint32_t node : nodeRing)
		{
			const Vec3& vertex = vertices[node];
			polygon1.push_back(Point(vertex.x(), vertex.y()));
		}
		//Insert the polygon into a constrained triangulation
		cdt.insert_constraint(polygon1.vertices_begin(), polygon1.vertices_end(), true);
	}

	//Mark facets that are inside the domain bounded by the polygon
	mark_domains(cdt);
	uint32_t indices = 0u;
	for (Face_handle f : cdt.finite_face_handles())
	{
		if (f->info().in_domain())
		{
			for (uint32_t i=0; i<3; i++)
			{
				Point vertex = f->vertex(int(i))->point();
				_mesh.vertices.push_back({float(vertex.x()), float(vertex.y()), zLevel});
				_mesh.normals.push_back(meshNormal);
				_mesh.indices.push_back(indices++);
			}
		}
	}
}
