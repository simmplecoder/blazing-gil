#include <boost/gil/typedefs.hpp>
#include <catch2/catch.hpp>
#include <cstdint>
#include <flash/core.hpp>
#include <type_traits>

namespace gil = boost::gil;

TEST_CASE("matrix to gray8 image conversion typecheck", "[to_matrix_channeled]")
{
    // gil::gray8_image_t input(16, 16, gil::gray8_pixel_t(13));
    // auto matrix = flash::to_matrix_channeled(gil::view(input));
    // STATIC_REQUIRE(std::is_same_v<blaze::StaticVector<std::uint8_t, 1>,
    //                               blaze::UnderlyingElement_t<decltype(matrix)>>);
    blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 3>> matrix(16, 16);
    auto image = flash::to_image<gil::rgb8_image_t>(matrix);
    STATIC_REQUIRE(std::is_same_v<decltype(image), gil::rgb8_image_t>);
}

TEST_CASE("matrix to gray8 image values check", "[to_matrix_channeled]")
{
    // gil::gray8_image_t input(16, 16, gil::gray8_pixel_t(13));
    // auto matrix = flash::to_matrix_channeled(gil::view(input));
    // blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 1>> expected(16, 16, {13});

    // REQUIRE(matrix == expected);
    const std::uint8_t value = 23;
    blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 1>> matrix(16, 16, {value});
    gil::gray8_image_t expected(16, 16, gil::gray8_pixel_t(value));

    auto result = flash::to_image<gil::gray8_image_t>(matrix);
    REQUIRE(result == expected);
}

TEST_CASE("matrix with differing values to gray8 image value check", "[to_matrix_channeled]")
{
    gil::gray8_image_t expected(16, 16, gil::gray8_pixel_t(13));
    auto view = gil::view(expected);
    view(0, 0)[0] = 0;
    view(1, 0)[0] = 1; // rows and cols are different for GIL vs Blaze

    blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 1>> input(16, 16, {13});
    input(0, 0)[0] = 0;
    input(0, 1)[0] = 1;
    auto result = flash::to_image<gil::gray8_image_t>(input);

    REQUIRE(result == expected);
}

TEST_CASE("matrix to rgb8 image typecheck", "[to_matrix_channeled]")
{
    gil::rgb8_pixel_t default_pixel(1, 2, 3);
    gil::rgb8_image_t input(16, 16, default_pixel);

    blaze::StaticVector<std::uint8_t, 3> default_vector({1, 2, 3});
    blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 3>> expected(16, 16, default_vector);

    auto result = flash::to_matrix_channeled(gil::view(input));
    STATIC_REQUIRE(std::is_same_v<blaze::StaticVector<std::uint8_t, 3>,
                                  blaze::UnderlyingElement_t<decltype(result)>>);
}

TEST_CASE("matrix to rgb8 image value check", "[to_matrix_channeled]")
{
    gil::rgb8_pixel_t default_pixel(1, 2, 3);
    gil::rgb8_image_t input(16, 16, default_pixel);

    blaze::StaticVector<std::uint8_t, 3> default_vector({1, 2, 3});
    blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 3>> expected(16, 16, default_vector);

    auto result = flash::to_matrix_channeled(gil::view(input));
    REQUIRE(result == expected);
}

TEST_CASE("matrix with differing values to rgb8 image value check", "[to_matrix_channeled]")
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