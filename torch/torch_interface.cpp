#include "torch_interface.h"
#include "../src/entry.hpp"

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
) {
  return callAccelerator(
          target_size,
          viewpoint.data_ptr<float>(),
          render_indices.data_ptr<int>(),
          nodes_for_render_indices.data_ptr<int>(),
          parent_indices.data_ptr<int>(),
          ts.data_ptr<float>(),
          kids.data_ptr<int>(),
          view_matrix.contiguous().cpu().data_ptr<float>(),
          proj_matrix.contiguous().cpu().data_ptr<float>()
  );
}