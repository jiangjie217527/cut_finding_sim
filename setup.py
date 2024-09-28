from setuptools import setup
from torch.utils.cpp_extension import CppExtension, BuildExtension
import os

# 获取当前文件的目录路径
current_dir = os.path.dirname(os.path.abspath(__file__))

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
            include_dirs=[os.path.join(os.path.dirname(os.path.abspath(__file__)), "dependencies/eigen/")],

            # 添加 ASan 的编译和链接标志
            extra_compile_args={
                "cxx": [
                    "-I" + current_dir,
                    ]
            },
        )
    ],
    cmdclass={
        'build_ext': BuildExtension
    }
)
