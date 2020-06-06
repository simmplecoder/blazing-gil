#include <blaze/math/dense/DynamicMatrix.h>
#include <blaze/math/dense/StaticVector.h>
#include <cstdint>
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <flash/core.hpp>

TEST_CASE("test remap_to - simple case", "[remap_test]")
{
    std::uint16_t value = 23;
    blaze::DynamicMatrix<std::uint16_t> input(16, 16, value);
    auto result = flash::remap_to<std::uint8_t>(input);
    REQUIRE(blaze::isZero(result));
}

TEST_CASE("test remap_to - slightly harder case", "[remap_test]")
{
    blaze::DynamicMatrix<std::uint16_t> input(16, 16);
    blaze::DynamicMatrix<std::uint8_t> expected(16, 16);
    std::uint8_t counter = 0;
    for (std::size_t i = 0; i < input.rows(); ++i)
    {
        for (std::size_t j = 0; j < input.columns(); ++j)
        {
            input(i, j) = counter;
            expected(i, j) = counter;
            ++counter;
        }
    }

    auto result = flash::remap_to<std::uint8_t>(input);
    REQUIRE(result == expected);
}

TEST_CASE("test remap_to_channeled - simple case", "[remap_test]")
{
    blaze::StaticVector<int, 3> default_vector({0, 1, 2});
    blaze::DynamicMatrix<blaze::StaticVector<int, 3>> matrix(2, 3, default_vector);

    blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 3>> expected(2, 3, {0, 0, 0});
    auto result = flash::remap_to_channeled<std::uint8_t>(matrix);
    REQUIRE(expected == result);
}

TEST_CASE("test remap_to_channeled - slightly harder case", "[remap_test]")
{
    blaze::StaticVector<int, 3> default_vector({0, 1, 2});
    blaze::DynamicMatrix<blaze::StaticVector<int, 3>> matrix(2, 3, default_vector);

    blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 3>> expected(2, 3, {10, 10, 10});
    auto result = flash::remap_to_channeled<std::uint8_t>(matrix, 10, 12);
    REQUIRE(expected == result);
}

TEST_CASE("test remap_to_channeled - linear case", "[remap_test]")
{
    // input range is from 0 to 15, so the length of the range is 16
    // the output range is from 0 to 255, so the length is 256
    // for every value in input, remapped value should be value * 16

    blaze::DynamicMatrix<blaze::StaticVector<int, 3>> matrix(2, 8, {0, 0, 0});
    int multiplier = 16;
    blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 3>> expected(2, 8, {0, 0, 0});

    for (unsigned int counter = 0; counter < 16; ++counter) {
        auto i = counter / 8;
        auto j = counter % 8;

        matrix(i, j)[0] = counter;
        expected(i, j)[0] = counter * multiplier;
    }

    auto result = flash::remap_to_channeled<std::uint8_t>(matrix);
    REQUIRE(result == expected);
}
