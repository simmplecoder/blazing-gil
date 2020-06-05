#include <catch2/catch.hpp>

#include <flash/core.hpp>

namespace gil = boost::gil;

template <typename T>
struct printer;

gil::gray8_pixel_t zero_gray8_pixel(0);

TEST_CASE("as_matrix typecheck for gray8_image", "[as_matrix]")
{
    gil::gray8_image_t image(16, 16, zero_gray8_pixel);
    auto view = gil::view(image);

    auto result = flash::as_matrix(view);
    using expected_t =
        blaze::CustomMatrix<gil::uint8_t, blaze::unaligned, blaze::unpadded, blaze::rowMajor>;
    STATIC_REQUIRE(std::is_same_v<decltype(result), expected_t>);
}

TEST_CASE("as_matrix gray8_image zero value_check", "[as_matrix]")
{
    gil::gray8_image_t image(16, 16, zero_gray8_pixel);
    auto view = gil::view(image);

    auto result = flash::as_matrix(view);
    REQUIRE(blaze::isZero(result));
}

TEST_CASE("as_matrix gray8_image modify matrix", "[as_matrix]")
{
    gil::gray8_image_t image(16, 16, zero_gray8_pixel);
    auto view = gil::view(image);

    auto result = flash::as_matrix(view);
    gil::gray8_image_t image2(16, 16, zero_gray8_pixel);
    auto expected_before_modify = gil::view(image2);
    REQUIRE(gil::equal_pixels(view, expected_before_modify));

    gil::uint8_t value = 23;
    result(1, 0) = value; // do not forget Blaze's different indexing
    REQUIRE(view(0, 1)[0] == value);

    std::uint8_t value2 = 40;
    result(3, 2) = value2;
    REQUIRE(view(2, 3)[0] == value2);
}

TEST_CASE("as_matrix gray8_image modify image", "[as_matrix]")
{
    gil::gray8_image_t image(16, 16, zero_gray8_pixel);
    auto view = gil::view(image);

    auto result = flash::as_matrix(view);
    auto expected_before_modify = blaze::zero<gil::uint8_t>(16, 16);
    REQUIRE(result == expected_before_modify);
    gil::uint8_t value = 23;
    view(1, 0)[0] = value;
    REQUIRE(result(0, 1) == value);

    std::uint8_t value2 = 40;
    view(3, 2)[0] = value2;
    REQUIRE(result(2, 3) == value2);
}

TEST_CASE("as_matrix typecheck for gray16_image", "[as_matrix]")
{
    gil::gray16_image_t image(16, 16);
    auto view = gil::view(image);

    auto result = flash::as_matrix(view);
    using expected_t =
        blaze::CustomMatrix<gil::uint16_t, blaze::unaligned, blaze::unpadded, blaze::rowMajor>;
    STATIC_REQUIRE(std::is_same_v<decltype(result), expected_t>);
}

gil::gray16_pixel_t zero_gray16_pixel(0);

TEST_CASE("as_matrix gray16_image zero value_check", "[as_matrix]")
{
    gil::gray16_image_t image(16, 16, zero_gray16_pixel);
    auto view = gil::view(image);

    auto result = flash::as_matrix(view);
    REQUIRE(blaze::isZero(result));
}

TEST_CASE("as_matrix gray16_image modify matrix", "[as_matrix]")
{
    gil::gray16_image_t image(16, 16, zero_gray16_pixel);
    auto view = gil::view(image);

    auto result = flash::as_matrix(view);
    gil::gray16_image_t image2(16, 16, zero_gray16_pixel);
    auto expected_before_modify = gil::view(image2);
    REQUIRE(gil::equal_pixels(view, expected_before_modify));

    gil::uint16_t value = 23;
    result(1, 0) = value; // do not forget Blaze's different indexing
    REQUIRE(view(0, 1)[0] == value);

    std::uint16_t value2 = 40;
    result(3, 2) = value2;
    REQUIRE(view(2, 3)[0] == value2);
}

TEST_CASE("as_matrix gray16_image modify image", "[as_matrix]")
{
    gil::gray16_image_t image(16, 16, zero_gray16_pixel);
    auto view = gil::view(image);

    auto result = flash::as_matrix(view);
    auto expected_before_modify = blaze::zero<gil::uint16_t>(16, 16);
    REQUIRE(result == expected_before_modify);
    gil::uint16_t value = 23;
    view(1, 0)[0] = value;
    REQUIRE(result(0, 1) == value);

    std::uint16_t value2 = 40;
    view(3, 2)[0] = value2;
    REQUIRE(result(2, 3) == value2);
}