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

    std::int64_t threshold = std::stoll(argv[4]);

    gil::gray8_image_t gray(input.dimensions());
    gil::copy_and_convert_pixels(gil::view(input), gil::view(gray));

    auto image = flash::to_matrix(gil::view(gray));
    blaze::DynamicMatrix<std::int16_t> mat(image);
    auto hessian_result = flash::hessian(mat);
    auto& hessian = hessian_result.determinants;
    hessian = blaze::map(hessian, [](std::int64_t x)
    {
        if (x >= 0)
            return x;
        else 
            return static_cast<std::int64_t>(0);
    });

    image = flash::remap_to<unsigned char>(hessian);
    const auto max_value = blaze::max(hessian);
    for (std::size_t i = 0; i < image.rows(); ++i)
    {
        for (std::size_t j = 0; j < image.columns(); ++j)
        {
            if (hessian(i, j) >= threshold)
            {
                gil::view(input)(j, i) = gil::rgb8_pixel_t(0, 255, 0);
            }
        }
    }

    std::cout << "Gradient range: " << blaze::max(hessian) << ' ' << blaze::min(hessian) << '\n'
              << "Final gray image range: " << static_cast<int>(blaze::max(image)) << ' ' << static_cast<int>(blaze::min(image)) << '\n';
    // gil::write_view(argv[2], gil::color_converted_view<gil::gray16_pixel_t>(gil::view(gray)), gil::png_tag{});
    gil::write_view(argv[2], gil::view(gray), gil::png_tag{});
    gil::write_view(argv[3], gil::view(input), gil::png_tag{});
}
