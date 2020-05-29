#include <catch2/catch.hpp>

#include <flash/core.hpp>
#include <blaze/Blaze.h>

namespace gil = boost::gil;

template <typename PixelVector>
void test_pixel_vector()
{
    using pixel_type = typename PixelVector::parent_t;
    using channel_type = typename gil::channel_type<pixel_type>::type;
    channel_type value = 23;
    constexpr auto num_channels = gil::num_channels<typename PixelVector::parent_t>{};
    {
        PixelVector zero_vector = blaze::generate(num_channels, [](std::size_t){return 0;});
        REQUIRE(blaze::isZero(zero_vector));
        REQUIRE(zero_vector.size() == num_channels);
    }
    
    {
        PixelVector vector = blaze::generate(num_channels, [value](){return value;});
        for (std::size_t i = 0; i < vector.size(); ++i)
        {
            REQUIRE(vector[i] == value);
        }
        REQUIRE(vector == value);
    }

    {
        PixelVector vector;
        for (std::size_t i = 0; i < vector.size(); ++i)
        {
            vector[i] = static_cast<channel_type>(i);
        }

        vector = blaze::map(vector, [](auto x) {return x * 2;});
        for (std::size_t i = 0; i < vector.size(); ++i)
        {
            REQUIRE(vector[i] == static_cast<channel_type>(2 * i));
        }
    }
}

TEST_CASE("rgb8 check")
{
    using pixel_vector = flash::pixel_vector<gil::rgb8_pixel_t>;
    test_pixel_vector<pixel_vector>();
}