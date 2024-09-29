#include "common.hpp"
#include "DCache.hpp"
#include "DRAM.hpp"
#include "pe.hpp"
#include "scheduler.hpp"
#include "types.hpp"

#include <cstdio>
#include <queue>
#include <vector>

void initStage();

int callAccelerator(float target_size,
                     float* viewpoint,
                     int* renderIndices,
                     int* nodesForRenderIndices,
                     int* parent_indices,
                     const float* view_matrix,
                     const float* proj_matrix);

