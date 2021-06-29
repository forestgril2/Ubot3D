#pragma once
#include <vector>
namespace Slicer {
class Layer;
using Layers = std::vector<Layer>;
class NaivePerimeterGenerator
{
public:
    NaivePerimeterGenerator(Layers& layers);
    void generate();
};
}
