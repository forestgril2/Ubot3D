#pragma once
#include <vector>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
namespace Slicer {

class Layer
{
public:
    using K = CGAL::Simple_cartesian<float>;
    using Point = K::Point_3;
    using Polyline_type = std::vector<Point>;
    using Polylines = std::list<Polyline_type>;

    Layer(const Polylines& polylines);
public:
    Polylines polylines;
};

}
