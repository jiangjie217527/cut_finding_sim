#include <torch/extension.h>
#include "torch/torch_interface.h"

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) {
  m.def("my_expand_to_size", &MyExpandToSize);
}