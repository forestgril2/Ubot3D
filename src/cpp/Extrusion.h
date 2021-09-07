#pragma once

#include <CommonDefs.h>

#define _USE_MATH_DEFINES
#include <math.h>

struct Extrusion
{
	using Point                = Eigen::Vector4f;                    /** Three dimensional coordinates +w == filament extrusion length. */
	using Path                 = std::vector<Point>;                 /** Points along the center of the filament path. */
	using LayerBottomDictEntry = std::pair<uint32_t, Real>;          /** First index of path with given layer bottom. */
	using AnnotationDictEntry  = std::pair<uint32_t, std::string*>;  /** First index of path with given annotation. */

	std::vector<Path>                 paths{Path{{0,0,0,0}}};
	std::vector<LayerBottomDictEntry> layerBottomsDict{{0u, 0.0f}};
	std::vector<AnnotationDictEntry>  annotationsDict;               /** Annotations for paths */

	size_t numPaths = 0;
	size_t numPathPointsMax = 0;

	bool isActive = false;
	float filamentCrossArea = 3.0f*3.0f*float(M_PI/4);   /** Filament cross-section area in mm^2 for filament cross section diameter 1.75mm */
};

