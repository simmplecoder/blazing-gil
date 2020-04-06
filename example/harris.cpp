#include <blaze/util/algorithms/Max.h>
#include <boost/gil/algorithm.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/typedefs.hpp>

#include <iostream>
#include <limits>

#include <core.hpp>
#include <numeric.hpp>

namespace gil = boost::gil;

int main(int argc, char* argv[])
{
    gil::rgb8_image_t input;
    gil::read_image(argv[1], input, gil::png_tag{});

    gil::gray8_image_t gray(input.dimensions());
    gil::copy_and_convert_pixels(gil::view(input), gil::view(gray));

    auto image = flash::to_matrix(gil::view(gray));
    blaze::DynamicMatrix<std::int16_t> mat(image);
    auto harris = flash::harris(mat, 0.04);

    image = flash::remap_to<unsigned char>(harris);
    const auto max_value = blaze::max(harris);
    for (std::size_t i = 0; i < image.rows(); ++i)
    {
        for (std::size_t j = 0; j < image.columns(); ++j)
        {
            if (image(i, j) >= 250)
            {
                gil::view(input)(j, i) = gil::rgb8_pixel_t(0, 255, 0);
            }
        }
    }

    std::cout << "Gradient range: " << blaze::max(harris) << ' ' << blaze::min(harris) << '\n'
              << "Final gray image range: " << static_cast<int>(blaze::max(image)) << ' ' << static_cast<int>(blaze::min(image)) << '\n';
    // gil::write_view(argv[2], gil::color_converted_view<gil::gray16_pixel_t>(gil::view(gray)), gil::png_tag{});
    gil::write_view(argv[2], gil::view(gray), gil::png_tag{});
    gil::write_view(argv[3], gil::view(input), gil::png_tag{});
}

