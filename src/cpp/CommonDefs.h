#pragma once

#include <qqml.h>

#include <functional>

#include <Eigen/Core>

#include <Chronograph.h>

using namespace Eigen;

using Real = float;
using Vertex = Eigen::Vector3f;
using Vertices = std::vector<Vertex>;
using Indices = std::vector<uint32_t>;
//using StrRefWrap = std::reference_wrapper<const std::string&>;

struct Extrusion
{
	using Point      = Eigen::Vector4f;                  /** Three dimensional coordinates +w == filament extrusion length. */
	using Path       = std::vector<Point>;               /** Points along the center of the filament path. */
	using Layer      = std::pair<uint32_t, Real>;        /** First index of path with given layer bottom. */
//	using Annotation = std::pair<uint32_t, StrRefWrap>; /** First index of path with given annotation. */

	std::vector<Path> paths{Path{{0,0,0,0}}};
	std::vector<Layer> layers{{0u, 0.0f}};
//	std::vector<Annotation> annotations; /** Additional annotations for paths*/

	size_t numPaths = 0;
	size_t numPathPointsMax = 0;

	bool isExtruding = false;

	float filamentCrossArea = 1.75f*1.75f*float(M_PI_4);  /** Filament cross-section area in mm^2 for filament cross section diameter 1.75mm */
};
