This library is a collection of image processing and computer vision algorithms. It uses Blaze for matrix computations and Boost.GIL for image input/output and encoding. 

## Installation

Since it uses both Blaze and GIL, those have to be installed on the system, along with all the dependencies. Here is the full list of dependencies

```
Boost 1.72 (header only libraries and filesystem)
libpng
libtiff
libtiffxx
libjpeg
libraw
Blaze (3.7+, preferably 3.8)
BLAS library (only Intel MKL is supported for now, but the rest will be added later)
```

The easiest way to consume the library is to pass `-DBOOST_ROOT=<Boost root>` and `-DMKLROOT=<intel root>/mkl` along with `-DUSE_CONAN`. That will install all of the image libraries for GIL. If `USE_CONAN` is not specified, those libraries have to be findable by `find_package` of CMake.


