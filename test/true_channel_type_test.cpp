#include <catch2/catch.hpp>

#include <flash/core.hpp>

namespace gil = boost::gil;

TEST_CASE("Identity case", "[true_channel_type]")
{
    STATIC_REQUIRE(std::is_same_v<std::uint8_t, flash::true_channel_type_t<std::uint8_t>>);
    STATIC_REQUIRE(std::is_same_v<std::uint16_t, flash::true_channel_type_t<std::uint16_t>>);
}

TEST_CASE("Underlying float case", "[true_channel_type]")
{
    STATIC_REQUIRE(std::is_same_v<float, flash::true_channel_type_t<gil::float32_t>>);
    STATIC_REQUIRE(std::is_same_v<double, flash::true_channel_type_t<gil::float64_t>>);
}