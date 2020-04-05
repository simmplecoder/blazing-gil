#include <blaze/math/AlignmentFlag.h>
#include <blaze/math/PaddingFlag.h>
#include <blaze/math/dense/CustomMatrix.h>
#include <blaze/math/expressions/DMatMapExpr.h>
#include <blaze/util/algorithms/Max.h>
#include <boost/gil/algorithm.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/typedefs.hpp>

#include <blaze/math/CustomMatrix.h>
#include <blaze/math/DynamicMatrix.h>
#include <blaze/math/Submatrix.h>

#include <iostream>
#include <limits>

namespace gil = boost::gil;

template <typename T>
using kernel2d = blaze::DynamicMatrix<T>;

template <typename T>
using image_matrix = blaze::CustomMatrix<T, blaze::unaligned, blaze::unpadded>;

template <typename T>
kernel2d<T> flip_kernel(const kernel2d<T>& source)
{
    auto m = source.rows();
    auto n = source.columns();    
    kernel2d<T> result(m, n);
    for (std::size_t i = 0; i < m; ++i)
    {
        for (std::size_t j = 0; j < n; ++j)
        {
            result(j, i) = source(m - i - 1, n - j - 1);
        }
    }

    return result;
}

template <typename T, typename U>
blaze::DynamicMatrix<T> convolve(const blaze::DynamicMatrix<T>& source, const kernel2d<U>& original_kernel)
{
    auto kernel = flip_kernel(original_kernel);
    auto m = source.rows();
    auto n = source.columns();

    blaze::DynamicMatrix<T> result(m, n, 0);
    auto kernel_size = kernel.rows();

    if (m < kernel_size || n < kernel_size)
    {
        return result;
    }

    for (std::size_t i = kernel_size; i < m - kernel_size; ++i)
    {
        for (std::size_t j = kernel_size; j < n - kernel_size; ++j)
        {
            auto current = blaze::submatrix(source, i, j, kernel_size, kernel_size);
            result(i, j) = blaze::sum(current % kernel);
        }
    }

    return result;
}

int main(int argc, char* argv[])
{
    gil::rgb8_image_t input;
    gil::read_image(argv[1], input, gil::png_tag{});

    gil::gray8_image_t gray(input.dimensions());
    gil::copy_and_convert_pixels(gil::view(input), gil::view(gray));

    image_matrix<unsigned char> image(reinterpret_cast<unsigned char*>(&gil::view(gray)(0, 0)), gray.height(), gray.width());
    blaze::DynamicMatrix<std::int16_t> mat(image);
    kernel2d<std::int16_t> kernel_x{{1, 0, -1}, {2, 0, -2}, {1, 0, -1}};
    auto dx = convolve(mat, kernel_x);

    kernel2d<std::int16_t> kernel_y{{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
    auto dy = convolve(mat, kernel_y);

    auto gradient = blaze::map(dx, dy, [](std::int16_t x, std::int16_t y)
    {
        return std::sqrt(x * x + y * y);
    });

    image = blaze::map(gradient, [](std::int16_t x) {
        return x / 4 * 1.159;
    });

    std::cout << static_cast<int>(blaze::max(image)) << ' ' << blaze::max(gradient) << '\n'
              << static_cast<int>(blaze::min(image)) << ' ' << blaze::min(gradient) << '\n';
    gil::write_view(argv[2], gil::view(gray), gil::png_tag{});
}

