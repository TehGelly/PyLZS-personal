from setuptools import Extension, setup
from os.path import join

setup(
    ext_modules=[
        Extension(
            name="LZS",  # as it would be imported
                               # may include packages/namespaces separated by `.`

            sources=[join("src","bitstream.c"), 
                     join("src","lzs_compress.c"), 
                     join("src","lzs_decompress.c"), 
                     join("src","lzsmodule.c")], # all sources are compiled into a single binary file,
            depends=[join("src","bitstream.h"),
                     join("src","lzs_compress.h"),
                     join("src","lzs_decompress.h"),
                     join("src","lzsmodule.h")],
            include_dirs = ["src"],
            py_limited_api = True,
            language='c'
        ),
    ]
)