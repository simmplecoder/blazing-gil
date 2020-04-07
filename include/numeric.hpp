#include <blaze/math/dense/DynamicMatrix.h>
#include <blaze/math/dense/StaticMatrix.h>
#include <blaze/math/expressions/DMatDMatAddExpr.h>
#include <blaze/math/expressions/DMatDMatSubExpr.h>
#include <blaze/math/expressions/DMatDMatSchurExpr.h>
#include <blaze/math/expressions/DMatScalarMultExpr.h>
#include <blaze/math/expressions/Matrix.h>
#include <cstdint>

#include <convolution.hpp>

namespace flash{
blaze::DynamicMatrix<std::int64_t> harris(const blaze::DynamicMatrix<std::int64_t>& image, double k)
{
    auto dx = flash::convolve(image, flash::sobel_x);
    auto dy = flash::convolve(image, flash::sobel_y);

    auto dx_2 = dx % dx;
    auto dy_2 = dy % dy;

    auto dxdy = flash::convolve(dx, flash::sobel_y);

    auto trace_2 = (dx_2 + dy_2) % (dx_2 + dy_2) * k;
    auto det = dx_2 % dy_2 - dxdy % dxdy;
    return det - trace_2;
} 
}