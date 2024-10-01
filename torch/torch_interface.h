#pragma once

#include <torch/extension.h>
#include <cstdio>
#include <tuple>
#include <string>

int MyExpandToSizeAndCalcWeight(
        float target_size,
        torch::Tensor &viewpoint,
        torch::Tensor &render_indices,
        torch::Tensor &nodes_for_render_indices,
        torch::Tensor &parent_indices,
        torch::Tensor &ts,
        torch::Tensor &kids,
        torch::Tensor &view_matrix,
        torch::Tensor &proj_matrix
);