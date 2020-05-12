#include <blaze/math/dense/DynamicMatrix.h>
#include <blaze/math/dense/StaticVector.h>
#include <cstdint>
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <flash/core.hpp>

TEST_CASE("test remap_to_channeled - simple case", "[to_matrix_channeled]")
{
    blaze::StaticVector<int, 3> default_vector({0, 1, 2});
    blaze::DynamicMatrix<blaze::StaticVector<int, 3>> matrix(2, 3, default_vector);

    blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 3>> expected(2, 3, {0, 0, 0});
    auto result = flash::remap_to_channeled<std::uint8_t>(matrix);
    REQUIRE(expected == result);
}

TEST_CASE("test remap_to_channeled - slightly harder case", "[to_matrix_channeled]")
{
    blaze::StaticVector<int, 3> default_vector({0, 1, 2});
    blaze::DynamicMatrix<blaze::StaticVector<int, 3>> matrix(2, 3, default_vector);

    blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 3>> expected(2, 3, {10, 10, 10});
    auto result = flash::remap_to_channeled<std::uint8_t>(matrix, 10, 12);
    REQUIRE(expected == result);
}

TEST_CASE("test remap_to_channeled - linear case", "[to_matrix_channeled]")
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
