#pragma once

#include <qqml.h>

#include <functional>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <Chronograph.h>

using namespace Eigen;

using Real = float;
using Vec3 = Eigen::Vector3f;
using Vertex = Vec3;
using Vertices = std::vector<Vertex>;
using Indices = std::vector<uint32_t>;
//using StrRefWrap = std::reference_wrapper<const std::string&>;
