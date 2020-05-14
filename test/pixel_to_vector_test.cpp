#include <catch2/catch.hpp>

#include <flash/core.hpp>

namespace gil = boost::gil;

TEST_CASE("single channel conversion typecheck", "[pixel_to_vector]")
{
    gil::gray8_pixel_t p;
    auto v = flash::pixel_to_vector(p);
    STATIC_REQUIRE(std::is_same_v<decltype(v), blaze::StaticVector<std::uint8_t, 1>>);
}

TEST_CASE("single channel zero conversion", "[pixel_to_vector]")
{
    gil::gray8_pixel_t p(0);
    auto v = flash::pixel_to_vector(p);
    REQUIRE(v[0] == 0);
}

TEST_CASE("single channel value conversion", "[pixel_to_vector]")
{
    gil::gray8_pixel_t p(23);
    auto v = flash::pixel_to_vector(p);
    REQUIRE(v[0] == 23);
}

TEST_CASE("three channel conversion typecheck", "[pixel_to_vector]")
{
    gil::rgb8_pixel_t p;
    auto v = flash::pixel_to_vector(p);
    STATIC_REQUIRE(std::is_same_v<decltype(v), blaze::StaticVector<std::uint8_t, 3>>);
}

TEST_CASE("simple zeroes multi-channel conversion", "[pixel_to_vector]")
{
    gil::rgb8_pixel_t p(0, 0, 0);
    auto vector = flash::pixel_to_vector(p);
    REQUIRE(vector[0] == 0);
    REQUIRE(vector[1] == 0);
    REQUIRE(vector[2] == 0);
}

TEST_CASE("same value multi channel conversion", "[pixel_to_vector]")
{
    auto value = std::uint8_t(23);
    gil::rgb8_pixel_t p(value, value, value);
    auto vector = flash::pixel_to_vector(p);
    REQUIRE(vector[0] == value);
    REQUIRE(vector[1] == value);
    REQUIRE(vector[2] == value);
}

TEST_CASE("distinct value multi channel conversion", "[pixel_to_vector]")
{
    auto value0 = std::uint8_t(23);
    auto value1 = std::uint8_t(11);
    auto value2 = std::uint8_t(1);
    gil::rgb8_pixel_t p(value0, value1, value2);
    auto vector = flash::pixel_to_vector(p);
    REQUIRE(vector[0] == value0);
    REQUIRE(vector[1] == value1);
    REQUIRE(vector[2] == value2);
}