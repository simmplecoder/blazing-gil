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
    REQUIRE(blaze::isZero(matrix_view));
}

TEST_CASE("rgba8_image_t different values as matrix valuecheck", "[as_matrix_channeled]")
{
    gil::rgba8_image_t image(16, 16, rgba8_zero_pixel);
    auto view = gil::view(image);

    std::uint8_t value(255);
    auto matrix_view = flash::as_matrix_channeled(view);
    for (std::size_t i = 0; i < matrix_view.rows(); ++i) {
        for (std::size_t j = 0; j < matrix_view.columns(); ++j) {
            matrix_view(i, j) = {static_cast<unsigned char>(i), static_cast<unsigned char>(j), value, 0};
        }
    }

    for (flash::signed_size i = 0; i < matrix_view.rows(); ++i) {
        for (flash::signed_size j = 0; j < matrix_view.columns(); ++j) {
            gil::rgba8_pixel_t expected_pixel(static_cast<unsigned char>(i), static_cast<unsigned char>(j), value, 0);
            REQUIRE(view(j, i) == expected_pixel);
        }
    }
}

gil::rgb32f_pixel_t rgb32f_zero_pixel(0, 0, 0);

TEST_CASE("rgb32f_image_t zero as matrix valuecheck", "[as_matrix_channeled]")
{
    gil::rgb32f_image_t image(16, 16, rgb32f_zero_pixel);
    auto view = gil::view(image);

    auto matrix_view = flash::as_matrix_channeled(view);
    REQUIRE(blaze::isZero(matrix_view));
}

TEST_CASE("rgb32f_image_t different values as matrix valuecheck", "[as_matrix_channeled]")
{
    gil::rgb32f_image_t image(16, 16, rgb32f_zero_pixel);
    auto view = gil::view(image);

    auto matrix_view = flash::as_matrix_channeled(view);
    float value = 0.125;
    for (std::size_t i = 0; i < matrix_view.rows(); ++i) {
        for (std::size_t j = 0; j < matrix_view.columns(); ++j) {
            matrix_view(i, j) = {static_cast<float>(i), static_cast<float>(j), value};
        }
    }

    for (flash::signed_size i = 0; i < matrix_view.rows(); ++i) {
        for (flash::signed_size j = 0; j < matrix_view.columns(); ++j) {
            gil::rgb32f_pixel_t expected_pixel(static_cast<float>(i), static_cast<float>(j), value);
            REQUIRE(view(j, i) == expected_pixel);
        }
    }
}