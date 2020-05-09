#include <boost/gil/image.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/typedefs.hpp>
#include <flash/core.hpp>
#include <iostream>

namespace gil = boost::gil;

int main()
{
    std::cout << "image to matrix\n";
    gil::rgb8_image_t image(5, 5, gil::rgb8_pixel_t(5, 5, 5));
    auto view = gil::view(image);

    view(0, 0) = gil::rgb8_pixel_t(0, 0, 0);
    // view(4, 4) = gil::rgb8_pixel_t(0, 0, 0);

    auto matrix = flash::to_matrix_channeled(view);
    std::cout << matrix.rows() << ' ' << matrix.columns() << '\n';

    for (std::size_t i = 0; i < matrix.rows(); ++i) {
        for (std::size_t j = 0; j < matrix.columns(); ++j) {
            std::cout << "m[" << i << "][" << j << "] = ("
                      << static_cast<int>(matrix(i, j)[0]) << ' '
                      << static_cast<int>(matrix(i, j)[1]) << ' '
                      << static_cast<int>(matrix(i, j)[2]) << ") ";
        }
        std::cout << '\n';
    }

    std::cout << "vector to pixel:\n";
    auto v = blaze::StaticVector<unsigned char, 3>{3, 4, 5};
    auto pixel = flash::vector_to_pixel<gil::rgb8_pixel_t>(v);
    std::cout << static_cast<int>(pixel[0]) << ' ' << static_cast<int>(pixel[1])
              << ' ' << static_cast<int>(pixel[2]) << '\n';

    std::cout << "matrix to image:\n";
    auto converted_image = flash::to_image<gil::rgb8_image_t>(matrix);
    std::cout << "width: " << converted_image.width()
              << " height: " << converted_image.height() << '\n';
    auto converted_view = gil::view(converted_image);
    for (flash::signed_size i = 0; i < converted_image.height(); ++i) {
        for (flash::signed_size j = 0; j < converted_image.width(); ++j) {
            std::cout << "pixel[" << i << "][" << j << "] = ("
                      << static_cast<int>(converted_view(i, j)[0]) << ' '
                      << static_cast<int>(converted_view(i, j)[1]) << ' '
                      << static_cast<int>(converted_view(i, j)[2]) << ") ";
        }
        std::cout << '\n';
    }
}