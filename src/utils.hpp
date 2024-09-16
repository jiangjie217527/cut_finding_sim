#ifndef HGS_UTILS_HPP_
#define HGS_UTILS_HPP_

#include "types.hpp"

float pointboxdist(const Box &box, const Point &viewpoint);
bool inbox(const Box &box, const Point &viewpoint);
float computeSize(const Box &box, const Point &viewpoint);

float4 transformPoint4x4(const float3& p, const float* matrix);
float3 transformPoint4x3(const float3& p, const float* matrix);
bool in_frustum(const Box &box, const float* view_matrix, const float* proj_matrix);

#endif // HGS_UTILS_HPP_