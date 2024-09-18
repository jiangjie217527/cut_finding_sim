from setuptools import setup
from torch.utils.cpp_extension import CUDAExtension, BuildExtension
import os
os.path.dirname(os.path.abspath(__file__))

setup(
    name="cut_finding_sim",
    ext_modules=[
        CUDAExtension(
            name="cut_finding_sim._C",
            sources=[
            "src/entry.cpp",
            "torch/torch_interface.cpp",
            "ext.cpp"],
            extra_compile_args={"cxx": ["-I" + os.path.join(os.path.dirname(os.path.abspath(__file__)))]}
            )
        ],
    cmdclass={
        'build_ext': BuildExtension
    }
)
