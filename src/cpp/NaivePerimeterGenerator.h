#pragma once
#include <vector>
#include "TriangleGeometry.h"
#include <clipper.hpp>

namespace Slicer {
class Layer;
using Layers = std::vector<Layer>;
class NaivePerimeterGenerator
{
public:
    NaivePerimeterGenerator(Layers& layers);
    void generate(TriangleGeometry& g);

    Layers& layers;
private:
    ClipperLib::Paths generate2();
    ClipperLib::Paths slicerPathsToClipperPaths(const Layer &layer);
    void debug_draw(std::vector<ClipperLib::Paths> paths, TriangleGeometry& g);
    ClipperLib::Paths offsetPath(ClipperLib::Paths layer,
                                 ClipperLib::Paths& outputPaths,
                                 float offset);
};
}
