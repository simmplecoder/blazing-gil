#include <catch2/catch.hpp>

#include <blaze/Blaze.h>
#include <flash/core.hpp>

TEST_CASE("simple pad case", "[pad]")
{
    std::size_t base_dimension = 8;
    for (std::size_t i = 1; i < 3; ++i)
    {
        blaze::DynamicMatrix<int> input(base_dimension, base_dimension, 0);
        blaze::DynamicMatrix<int> expected(base_dimension + i * 2, base_dimension + i * 2, 0);
        auto result = flash::pad(input, i, 0);
        REQUIRE(result.rows() == expected.rows());
        REQUIRE(result.columns() == expected.columns());
        REQUIRE(result == expected);
    }
}

TEST_CASE("harder pad case", "[pad]")
{
    std::size_t base_dimension = 8;
    int value = 23;
    int pad_value = 11;
    for (std::size_t i = 1; i < 3; ++i)
    {
        blaze::DynamicMatrix<int> input(base_dimension, base_dimension, value);
        auto result = flash::pad(input, i, pad_value);
        // check surrounding values first
        // upper rows
        REQUIRE(blaze::submatrix(result, 0, 0, i, result.columns()) == pad_value);
        // left columns
        REQUIRE(blaze::submatrix(result, 0, 0, result.rows(), i) == pad_value);
        // right columns
        REQUIRE(blaze::submatrix(result, 0, input.columns() + i, result.rows(), i) == pad_value);
        // lower rows
        REQUIRE(blaze::submatrix(result, input.rows() + i, 0, i, result.columns()) == pad_value);

        //check center
        REQUIRE(blaze::submatrix(result, i, i, input.rows(), input.columns()) == value);
    }
}

