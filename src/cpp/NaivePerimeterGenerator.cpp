#include "NaivePerimeterGenerator.h"

#include "Layer.h"

#include <QVector3D>

using namespace Slicer;

const auto multiplier = 100000;
const auto numPerimeters = 3;
const auto perimeterOffset = -0.5f;

NaivePerimeterGenerator::NaivePerimeterGenerator(Layers &layers)
 : layers(layers)
{

}

void NaivePerimeterGenerator::generate(TriangleGeometry& g)
{
    int layerCnt = 0;

    for(auto layerIt = layers.begin();
        layerIt != layers.end(); ++layerIt)
    {
        layerCnt++;
        if(layerCnt != 2) continue;

        auto paths = slicerPathsToClipperPaths(*layerIt);


        std::vector<ClipperLib::Paths> debugPaths;
        ClipperLib::Paths out = paths;
        for(int i=0;i<numPerimeters;i++)
        {
            offsetPath(out, out, perimeterOffset);
            debugPaths.push_back(out);
        }

        debug_draw(debugPaths, g);
    }
}

ClipperLib::Paths NaivePerimeterGenerator::offsetPath(ClipperLib::Paths inputPaths,
                                                      ClipperLib::Paths& outputPaths,
                                                      float offset)
{
    ClipperLib::ClipperOffset clipperOffset;

    clipperOffset.AddPaths(inputPaths, ClipperLib::jtMiter, ClipperLib::etClosedPolygon);
    clipperOffset.Execute(outputPaths, multiplier * offset);

    return outputPaths;
}

ClipperLib::Paths NaivePerimeterGenerator::slicerPathsToClipperPaths(const Layer& layer)
{
    ClipperLib::Paths paths;

    for(auto const& polyline : layer.polylines)
    {
        ClipperLib::Path path;
        for(auto const& p : polyline)
        {
            auto x = static_cast<int>(std::round(p.x()*multiplier));
            auto y = static_cast<int>(std::round(p.y()*multiplier));
            path << ClipperLib::IntPoint(x, y);
        }

        paths.push_back(path);
    }

    return paths;
}

void NaivePerimeterGenerator::debug_draw(std::vector<ClipperLib::Paths> paths,
                                   TriangleGeometry& g)
{

    for(auto const& pp : paths)
    {
        QVector<QVector3D> offsetBoundary;
        for (auto const& path : pp)
        {
            size_t vertCnt = 0;
            for (auto const& point : path)
            {
                auto p = QVector3D{float(double(point.X)/multiplier),
                        float(double(point.Y)/multiplier), 0.0f};

                offsetBoundary.emplace_back(p);

                if(vertCnt > 0)
                {
                    offsetBoundary.emplace_back(p);
                }

                if(vertCnt == path.size()-1)
                {
                    offsetBoundary.emplace_back(QVector3D{float(double(path[0].X)/multiplier),
                                                     float(double(path[0].Y)/multiplier), 0.0f});
                }

                vertCnt++;
            }

            g._triangleIslandBoundaries.emplace_back(offsetBoundary);
        }
    }

    emit g.raftGeometriesChanged();
}
