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

template <typename PixelType, typename ImageType>
void pixel4_uint8_zeroes_test()
{
    PixelType zero_pixel = PixelType(0, 0, 0, 0);
    ImageType image(16, 16, zero_pixel);
    auto view = gil::view(image);

    auto matrix_view = flash::as_matrix_channeled(view);
    REQUIRE(blaze::isZero(matrix_view));
}

template <typename PixelType, typename ImageType>
void pixel4_uint8_different_values_test()
{
    PixelType zero_pixel = PixelType(0, 0, 0, 0);
    ImageType image(16, 16, zero_pixel);
    auto view = gil::view(image);

    auto matrix_view = flash::as_matrix_channeled(view);
    std::uint8_t value(255);
    for (std::size_t i = 0; i < matrix_view.rows(); ++i) {
        for (std::size_t j = 0; j < matrix_view.columns(); ++j) {
            matrix_view(i, j) = {
                static_cast<unsigned char>(i), static_cast<unsigned char>(j), value, 0};
        }
    }

    for (std::size_t i = 0; i < matrix_view.rows(); ++i) {
        for (std::size_t j = 0; j < matrix_view.columns(); ++j) {
            gil::rgba8_pixel_t expected_pixel(
                static_cast<unsigned char>(i), static_cast<unsigned char>(j), value, 0);
            REQUIRE(view(j, i) == expected_pixel);
        }
    }
}

template <typename PixelType, typename ImageType>
void pixel4_float32_zeroes_test()
{
    PixelType zero_pixel = PixelType(0, 0, 0, 0);
    ImageType image(16, 16, zero_pixel);
    auto view = gil::view(image);

    auto matrix_view = flash::as_matrix_channeled(view);
    REQUIRE(blaze::isZero(matrix_view));
}

template <typename PixelType, typename ImageType>
void pixel4_float32_different_values_test()
{
    PixelType zero_pixel = PixelType(0, 0, 0, 0);
    ImageType image(16, 16, zero_pixel);
    auto view = gil::view(image);

    auto matrix_view = flash::as_matrix_channeled(view);
    float value = 0.125;
    for (std::size_t i = 0; i < matrix_view.rows(); ++i) {
        for (std::size_t j = 0; j < matrix_view.columns(); ++j) {
            matrix_view(i, j) = {static_cast<float>(i), static_cast<float>(j), value, 0.0f};
        }
    }

    for (std::size_t i = 0; i < matrix_view.rows(); ++i) {
        for (std::size_t j = 0; j < matrix_view.columns(); ++j) {
            PixelType expected_pixel(static_cast<float>(i), static_cast<float>(j), value, 0.0f);
            REQUIRE(view(j, i) == expected_pixel);
        }
    }
}

template <typename PixelType, typename ImageType>
void pixel3_float32_zeroes_test()
{
    PixelType zero_pixel = PixelType(0, 0, 0);
    ImageType image(16, 16, zero_pixel);
    auto view = gil::view(image);

    auto matrix_view = flash::as_matrix_channeled(view);
    REQUIRE(blaze::isZero(matrix_view));
}

template <typename PixelType, typename ImageType>
void pixel3_float32_different_values_test()
{
    PixelType zero_pixel = PixelType(0, 0, 0);
    ImageType image(16, 16, zero_pixel);
    auto view = gil::view(image);

    auto matrix_view = flash::as_matrix_channeled(view);
    float value = 0.125;
    for (std::size_t i = 0; i < matrix_view.rows(); ++i) {
        for (std::size_t j = 0; j < matrix_view.columns(); ++j) {
            matrix_view(i, j) = {static_cast<float>(i), static_cast<float>(j), value};
        }
    }

    for (std::size_t i = 0; i < matrix_view.rows(); ++i) {
        for (std::size_t j = 0; j < matrix_view.columns(); ++j) {
            PixelType expected_pixel(static_cast<float>(i), static_cast<float>(j), value);
            REQUIRE(view(j, i) == expected_pixel);
        }
    }
}

TEST_CASE("check rgba8 as_matrix_channeled", "[as_matrix_channeled]")
{
    using pixel_type = gil::rgba8_pixel_t;
    using image_type = gil::rgba8_image_t;
    pixel4_uint8_zeroes_test<pixel_type, image_type>();
    pixel4_uint8_different_values_test<pixel_type, image_type>();
}

TEST_CASE("check rgb32f as_matrix_channeled", "[as_matrix_channeled]")
{
    using pixel_type = gil::rgb32f_pixel_t;
    using image_type = gil::rgb32f_image_t;
    pixel3_float32_zeroes_test<pixel_type, image_type>();
    pixel3_float32_different_values_test<pixel_type, image_type>();
}

TEST_CASE("check rgba32f as_matrix_channeled", "[as_matrix_channeled]")
{
    using pixel_type = gil::rgba32f_pixel_t;
    using image_type = gil::rgba32f_image_t;
    pixel4_float32_zeroes_test<pixel_type, image_type>();
    pixel4_float32_different_values_test<pixel_type, image_type>();
}

TEST_CASE("check cmyk8 as_matrix_channeled", "[as_matrix_channeled]")
{
    using pixel_type = gil::cmyk8_pixel_t;
    using image_type = gil::cmyk8_image_t;
    pixel4_uint8_zeroes_test<pixel_type, image_type>();
    pixel4_uint8_different_values_test<pixel_type, image_type>();
}

TEST_CASE("check cmyk32f as_matrix_channeled", "[as_matrix_channeled]")
{
    using pixel_type = gil::cmyk32f_pixel_t;
    using image_type = gil::cmyk32f_image_t;
    pixel4_float32_zeroes_test<pixel_type, image_type>();
    pixel4_float32_different_values_test<pixel_type, image_type>();
}