#include <blaze/Blaze.h>
#include <blaze/math/AlignmentFlag.h>
#include <blaze/math/PaddingFlag.h>
#include <boost/gil/image.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/typedefs.hpp>
#include <flash/pixel_compat.hpp>

#include <cstdint>
#include <iostream>

namespace gil = boost::gil;

int main()
{
    std::uint8_t value = 23;
    gil::rgb8_pixel_t default_pixel(0, value, 1);
    gil::rgb8_image_t image(16, 16, default_pixel);
    auto view = gil::view(image);

    using pixel_vector_t = pixel_vector_type<gil::rgb8_pixel_t>::type;

    blaze::CustomMatrix<pixel_vector_t, blaze::unaligned, blaze::unpadded> matrix(
        reinterpret_cast<pixel_vector_t*>(&view(0, 0)), 16, 16);
    matrix = matrix % matrix;
    for (std::size_t i = 0; i < matrix.rows(); ++i) {
        for (std::size_t j = 0; j < matrix.columns(); ++j) {

            std::cout << "m[" << i << "][" << j << "] = ("
                      << static_cast<unsigned int>(matrix(i, j)[0]) << ", "
                      << static_cast<unsigned int>(matrix(i, j)[1]) << ", "
                      << static_cast<unsigned int>(matrix(i, j)[2]) << ")\n";
        }
    }
}