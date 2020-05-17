#include <catch2/catch.hpp>

#include <flash/core.hpp>

namespace gil = boost::gil;

TEST_CASE("1d vector to pixel typecheck", "[vector_to_pixel]")
{
    blaze::StaticVector<std::uint8_t, 1> v({0});
    auto p = flash::vector_to_pixel<gil::gray8_pixel_t>(v);

    STATIC_REQUIRE(std::is_same_v<decltype(p), gil::gray8_pixel_t>);
}

TEST_CASE("1d zero vector to pixel value check", "[vector_to_pixel]")
{
    blaze::StaticVector<std::uint8_t, 1> v({0});
    auto p = flash::vector_to_pixel<gil::gray8_pixel_t>(v);

    REQUIRE(p[0] == 0);
}

TEST_CASE("1d vector to pixel value check", "[vector_to_pixel]")
{
    blaze::StaticVector<std::uint8_t, 1> v({23});
    auto p = flash::vector_to_pixel<gil::gray8_pixel_t>(v);

    REQUIRE(p[0] == 23);
}

TEST_CASE("3d vector to pixel typecheck", "[vector_to_pixel]")
{
    blaze::StaticVector<std::uint8_t, 3> v({0});
    auto p = flash::vector_to_pixel<gil::rgb8_pixel_t>(v);

    STATIC_REQUIRE(std::is_same_v<decltype(p), gil::rgb8_pixel_t>);
}

TEST_CASE("3d zero vector to pixel value check", "[vector_to_pixel]")
{
    blaze::StaticVector<std::uint8_t, 3> v({0, 0, 0});
    auto p = flash::vector_to_pixel<gil::rgb8_pixel_t>(v);

    REQUIRE(p[0] == 0);
    REQUIRE(p[1] == 0);
    REQUIRE(p[2] == 0);
}

TEST_CASE("3d same value vector to pixel value check", "[vector_to_pixel]")
{
    std::uint8_t value = 23;
    blaze::StaticVector<std::uint8_t, 3> v({value, value, value});

    auto p = flash::vector_to_pixel<gil::rgb8_pixel_t>(v);

    REQUIRE(p[0] == value);
    REQUIRE(p[1] == value);
    REQUIRE(p[2] == value);
}

TEST_CASE("3d distinct value vector to pixel value check", "[vector_to_pixel]")
{
    std::uint8_t value0 = 23;
    std::uint8_t value1 = 11;
    std::uint8_t value2 = 1;
    blaze::StaticVector<std::uint8_t, 3> v({value0, value1, value2});
    auto p = flash::vector_to_pixel<gil::rgb8_pixel_t>(v);

    REQUIRE(p[0] == value0);
    REQUIRE(p[1] == value1);
    REQUIRE(p[2] == value2);
}