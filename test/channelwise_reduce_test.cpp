#include <algorithm>
#include <array>
#include <catch2/catch.hpp>

#include <blaze/Blaze.h>
#include <flash/core.hpp>
#include <random>

TEST_CASE("simple min max case", "[channelwise_reduce]")
{
    auto matrix = blaze::evaluate(blaze::generate(
        16, 16, [](std::size_t, std::size_t) { return blaze::StaticVector<int, 3>(0); }));
    auto the_min = flash::channelwise_min(matrix);
    auto the_max = flash::channelwise_max(matrix);
    REQUIRE(the_min == 0);
    REQUIRE(the_max == 0);
}

TEST_CASE("slightly harder min max case", "[channelwise_reduce]")
{
    int min_value = -11;
    int max_value = 11;
    auto matrix = blaze::evaluate(blaze::generate(
        16, 16, [](std::size_t, std::size_t) { return blaze::StaticVector<int, 3>(0); }));
    matrix(0, 0) = {max_value, max_value, min_value};
    matrix(10, 10) = {min_value, min_value, 0};
    matrix(2, 2) = {min_value, max_value, max_value};
    auto the_min = flash::channelwise_min(matrix);
    auto the_max = flash::channelwise_max(matrix);
    REQUIRE(the_min == blaze::StaticVector<int, 3>({min_value, min_value, min_value}));
    REQUIRE(the_max == blaze::StaticVector<int, 3>({max_value, max_value, max_value}));
}

TEST_CASE("random min max case", "[channelwise_reduce]")
{
    std::mt19937 twister(std::random_device{}());
    std::uniform_int_distribution<int> dist;
    using input_elem_type = std::array<int, 3>;
    using input_vector_type = std::vector<input_elem_type>;
    input_vector_type input(16 * 16);
    std::generate(input.begin(), input.end(), [&twister, &dist]() mutable {
        return input_elem_type{dist(twister), dist(twister), dist(twister)};
    });

    input_elem_type expected_min = input[0];
    input_elem_type expected_max = input[0];
    for (std::size_t i = 1; i < input.size(); ++i) {
        for (std::size_t channel = 0; channel < 3; ++channel) {
            if (expected_min[channel] > input[i][channel]) {
                expected_min[channel] = input[i][channel];
            }

            if (expected_max[channel] < input[i][channel]) {
                expected_max[channel] = input[i][channel];
            }
        }
    }

    auto matrix = blaze::generate(
        16, 16, [&input](std::size_t i, std::size_t j) { return input[i * 16 + j]; });
    auto the_min = flash::channelwise_min(matrix);
    auto the_max = flash::channelwise_max(matrix);

    for (std::size_t i = 0; i < 3; ++i) {
        REQUIRE(expected_min[i] == the_min[i]);
        REQUIRE(expected_max[i] == the_max[i]);
    }
}