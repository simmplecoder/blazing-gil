#include <blaze/math/AlignmentFlag.h>
#include <blaze/math/PaddingFlag.h>
#include <blaze/math/StorageOrder.h>
#include <blaze/math/shims/IsZero.h>
#include <boost/gil/typedefs.hpp>
#include <catch2/catch.hpp>

#include <flash/core.hpp>
#include <iostream>

namespace gil = boost::gil;

gil::rgba8_pixel_t rgba8_zero_pixel(0, 0, 0, 0);

TEST_CASE("rgba8_image_t as matrix typecheck", "[as_matrix_channeled]")
{
    gil::rgba8_image_t image(16, 16, rgba8_zero_pixel);
    auto view = gil::view(image);

    auto matrix_view = flash::as_matrix_channeled(view);
    using expected_t = blaze::CustomMatrix<
        blaze::StaticVector<std::uint8_t, 4, blaze::rowMajor, blaze::unaligned, blaze::unpadded>,
        blaze::unaligned,
        blaze::unpadded,
        blaze::rowMajor>;
    STATIC_REQUIRE(std::is_same_v<decltype(matrix_view), expected_t>);
}

TEST_CASE("rgba8_image_t zero as matrix valuecheck", "[as_matrix_channeled]")
{
    gil::rgba8_image_t image(16, 16, rgba8_zero_pixel);
    auto view = gil::view(image);

    auto matrix_view = flash::as_matrix_channeled(view);
    for (std::size_t i = 0; i < matrix_view.rows(); ++i) {
        for (std::size_t j = 0; j < matrix_view.columns(); ++j) {
            // std::uint8_t is char which will print garbage
            const blaze::StaticVector<std::uint16_t, 4> element = matrix_view(i, j);
            std::cout << "matrix_view[" << i << "][" << j << "] = (" << element[0] << ", "
                      << element[1] << ", " << element[2] << ")\n";
        }
    }
    REQUIRE(blaze::isZero(matrix_view));
}