#include <blaze/util/algorithms/Max.h>
#include <boost/gil/algorithm.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/typedefs.hpp>

#include <iostream>
#include <limits>

#include <core.hpp>
#include <convolution.hpp>

namespace gil = boost::gil;

int main(int argc, char* argv[])
{
    gil::rgb8_image_t input;
    gil::read_image(argv[1], input, gil::png_tag{});

    gil::gray8_image_t gray(input.dimensions());
    gil::copy_and_convert_pixels(gil::view(input), gil::view(gray));

    auto image = flash::to_matrix(gil::view(gray));
    blaze::DynamicMatrix<std::int16_t> mat(image);
    auto dx = flash::convolve(mat, flash::sobel_x);

    flash::kernel2d<std::int16_t> kernel_y{{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
    auto dy = flash::convolve(mat, flash::sobel_y);

    auto gradient = blaze::map(dx, dy, [](std::int16_t x, std::int16_t y)
    {
        return std::sqrt(x * x + y * y);
    });

    image = flash::remap_to<unsigned char>(gradient);

    std::cout << static_cast<int>(blaze::max(image)) << ' ' << blaze::max(gradient) << '\n'
              << static_cast<int>(blaze::min(image)) << ' ' << blaze::min(gradient) << '\n';
    gil::write_view(argv[2], gil::view(gray), gil::png_tag{});
}

