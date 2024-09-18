#pragma once
#include <torch/extension.h>
#include <cstdio>
#include <tuple>
#include <string>

int MyExpandToSize(
    float target_size,
    torch::Tensor& viewpoint,
    torch::Tensor& render_indices,
    torch::Tensor& parent_indices,
    torch::Tensor& view_matrix,
    torch::Tensor& proj_matrix
);