#include "pe.hpp"

InnerTask::InnerTask(int offset) : offset(offset) {}

PE::PE(std::vector<int>& render_indices,
                  std::vector<int>& parent_indices) : render_indices(render_indices), parent_indices(parent_indices) {
    for (int i = 0; i < PipelineStage; ++i) {
        inner_tasks[i] = InnerTask(i);
    }
}