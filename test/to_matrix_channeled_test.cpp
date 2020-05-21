#include <blaze/math/dense/DynamicMatrix.h>
#include <blaze/math/dense/StaticVector.h>
#include <blaze/math/typetraits/UnderlyingElement.h>
#include <catch2/catch.hpp>
#include <cstdint>
#include <flash/core.hpp>
#include <type_traits>

namespace gil = boost::gil;

TEST_CASE("gray8 image to matrix conversion typecheck", "[to_matrix_channeled]")
{
    gil::gray8_image_t input(16, 16, gil::gray8_pixel_t(13));
    auto matrix = flash::to_matrix_channeled(gil::view(input));
    STATIC_REQUIRE(std::is_same_v<blaze::StaticVector<std::uint8_t, 1>,
                                  blaze::UnderlyingElement_t<decltype(matrix)>>);
}

TEST_CASE("gray8 image to matrix values check", "[to_matrix_channeled]")
{
    gil::gray8_image_t input(16, 16, gil::gray8_pixel_t(13));
    auto matrix = flash::to_matrix_channeled(gil::view(input));
    blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 1>> expected(16, 16, {13});

    REQUIRE(matrix == expected);
}

TEST_CASE("gray8 image with differing values to matrix value check", "[to_matrix_channeled]")
{
    gil::gray8_image_t input(16, 16, gil::gray8_pixel_t(13));
    auto view = gil::view(input);
    view(0, 0)[0] = 0;
    view(1, 0)[0] = 1; // rows and cols are different for GIL vs Blaze

    auto matrix = flash::to_matrix_channeled(gil::view(input));
    blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 1>> expected(16, 16, {13});
    expected(0, 0)[0] = 0;
    expected(0, 1)[0] = 1;

    REQUIRE(matrix == expected);
}

TEST_CASE("rgb8 image to matrix typecheck", "[to_matrix_channeled]")
{
    gil::rgb8_pixel_t default_pixel(1, 2, 3);
    gil::rgb8_image_t input(16, 16, default_pixel);

    blaze::StaticVector<std::uint8_t, 3> default_vector({1, 2, 3});
    blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 3>> expected(16, 16, default_vector);

    auto result = flash::to_matrix_channeled(gil::view(input));
    STATIC_REQUIRE(std::is_same_v<blaze::StaticVector<std::uint8_t, 3>,
                                  blaze::UnderlyingElement_t<decltype(result)>>);
}

TEST_CASE("rgb8 image to matrix value check", "[to_matrix_channeled]")
{
    gil::rgb8_pixel_t default_pixel(1, 2, 3);
    gil::rgb8_image_t input(16, 16, default_pixel);

    blaze::StaticVector<std::uint8_t, 3> default_vector({1, 2, 3});
    blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 3>> expected(16, 16, default_vector);

    auto result = flash::to_matrix_channeled(gil::view(input));
    REQUIRE(result == expected);
}

TEST_CASE("rgb8 image with differing values to matrix value check", "[to_matrix_channeled]")
{
    gil::rgb8_pixel_t default_pixel(1, 2, 3);
    gil::rgb8_image_t input(16, 16, default_pixel);
    auto view = gil::view(input);
    view(0, 0) = gil::rgb8_pixel_t(10, 20, 30);
    view(1, 0) = gil::rgb8_pixel_t(50, 50, 50);

    blaze::StaticVector<std::uint8_t, 3> default_vector({1, 2, 3});
    blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 3>> expected(16, 16, default_vector);
    expected(0, 0) = {10, 20, 30};
    expected(0, 1) = {50, 50, 50};

    auto result = flash::to_matrix_channeled(gil::view(input));
    REQUIRE(result == expected);
}