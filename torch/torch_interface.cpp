#include "torch_interface.h"
#include "../src/entry.hpp"

int MyExpandToSize(
        float target_size,
        torch::Tensor& viewpoint,
        torch::Tensor& render_indices,
        torch::Tensor& parent_indices,
        torch::Tensor& view_matrix,
        torch::Tensor& proj_matrix
) {
  return callAccelerator(
          target_size,
          viewpoint.data_ptr<float>(),
          render_indices.data_ptr<int>(),
          parent_indices.data_ptr<int>(),
          view_matrix.contiguous().cpu().data_ptr<float>(),
          proj_matrix.contiguous().cpu().data_ptr<float>()
  );
}