#include "common.hpp"
#include "utils.hpp"
#include "types.hpp"
#include <cmath>

bool inbox(const Box &box, const Point &viewpoint) {
    bool inside = true;

//    std::cout << "box = (" << box.minn[0] << ", " << box.minn[1] << ", " << box.minn[2] << ") - (" << box.maxx[0] << ", " << box.maxx[1] << ", " << box.maxx[2] << ")" << std::endl;
    for (int i = 0; i < 3; ++i) {
        inside &= box.minn[i] <= viewpoint[i] && viewpoint[i] <= box.maxx[i];
    }

    return inside;
}

float pointboxdist(const Box &box, const Point &viewpoint) {
    Point closest = {
        std::max(box.minn[0], std::min(viewpoint[0], box.maxx[0])),
        std::max(box.minn[1], std::min(viewpoint[1], box.maxx[1])),
        std::max(box.minn[2], std::min(viewpoint[2], box.maxx[2]))
    };
    
    Point diff = {
        closest[0] - viewpoint[0],
        closest[1] - viewpoint[1],
        closest[2] - viewpoint[2]
    };

//    std::cout << "[INFO] diff = (" << diff[0] << ", " << diff[1] << ", " << diff[2] << ")\n";

    return std::sqrt(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]);
}

float computeSize(const Box &box, const Point &viewpoint) {
    if (inbox(box, viewpoint)) {
        return __FLT_MAX__;
    }

    float min_dist = pointboxdist(box, viewpoint);

    return box.minn[3] / min_dist;
}

float4 transformPoint4x4(const float3& p, const float* matrix) {
    float4 transformed = {
		matrix[0] * p[0] + matrix[4] * p[1] + matrix[8] * p[2] + matrix[12],
        matrix[1] * p[0] + matrix[5] * p[1] + matrix[9] * p[2] + matrix[13],
        matrix[2] * p[0] + matrix[6] * p[1] + matrix[10] * p[2] + matrix[14],
        matrix[3] * p[0] + matrix[7] * p[1] + matrix[11] * p[2] + matrix[15]
    };

	return transformed;
}

float3 transformPoint4x3(const float3& p, const float* matrix) {
    float3 transformed = {
        matrix[0] * p[0] + matrix[4] * p[1] + matrix[8] * p[2] + matrix[12],
        matrix[1] * p[0] + matrix[5] * p[1] + matrix[9] * p[2] + matrix[13],
        matrix[2] * p[0] + matrix[6] * p[1] + matrix[10] * p[2] + matrix[14]
    };

    return transformed;
}

bool in_frustum(const Box &box, const float* view_matrix, const float* proj_matrix) {
    bool inside = false;
    float3 corners[8] = {
        {box.minn[0], box.minn[1], box.minn[2]},
        {box.minn[0], box.minn[1], box.maxx[2]},
        {box.minn[0], box.maxx[1], box.minn[2]},
        {box.minn[0], box.maxx[1], box.maxx[2]},
        {box.maxx[0], box.minn[1], box.minn[2]},
        {box.maxx[0], box.minn[1], box.maxx[2]},
        {box.maxx[0], box.maxx[1], box.minn[2]},
        {box.maxx[0], box.maxx[1], box.maxx[2]}
    };

    for (int i = 0; i < 8; ++i) {
        float4 p_hom = transformPoint4x4(corners[i], proj_matrix);
        float p_w = 1.0f / (p_hom[3] + 0.0000001f);
        float3 p_view = transformPoint4x3(corners[i], view_matrix);
        if (p_view[2] > 0.2f) {
            inside = true;
            break;
        }
    }

//    std::cout << "[INFO] inside = " << inside << "\n";

    return inside;
}