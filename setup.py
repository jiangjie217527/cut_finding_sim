from setuptools import setup
from torch.utils.cpp_extension import CppExtension, BuildExtension
import os

setup(
    name="cut_finding_sim",
    ext_modules=[
        CppExtension(
            name="cut_finding_sim._C",
            sources=[
                "src/entry.cpp",
                "torch/torch_interface.cpp",
                "src/DCache.cpp",
                "src/DRAM.cpp",
                "src/pe.cpp",
                "src/scheduler.cpp",
                "src/utils.cpp",
                "ext.cpp"
            ],
            extra_compile_args={"cxx": ["-I" + os.path.join(os.path.dirname(os.path.abspath(__file__)))]}
        )
    ],
    cmdclass={
        'build_ext': BuildExtension
    }
)
