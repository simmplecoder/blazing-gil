#include <boost/gil/algorithm.hpp>
#include <boost/gil/typedefs.hpp>
#include <catch2/catch.hpp>

#include <flash/core.hpp>

#include <limits>
#include <random>
#include <type_traits>

namespace gil = boost::gil;

template <typename ImageType>
void test_to_matrix_type()
{
    using pixel_type = typename ImageType::value_type;
    using channel_type = typename gil::channel_type<pixel_type>::type;
    constexpr auto num_channels = gil::num_channels<ImageType>{};

    pixel_type zero_pixel{};
    for (std::size_t i = 0; i < num_channels; ++i) {
        zero_pixel[i] = 0;
    }

    ImageType zero_image(16, 16, zero_pixel);
    auto view = gil::view(zero_image);

    for (std::size_t i = 0; i < num_channels; ++i) {
        auto result = flash::to_matrix(view, i);
        REQUIRE(blaze::isZero(result));
    }

    gil::transform_pixels(view, view, [num_channels](auto) {
        pixel_type pixel;
        for (std::size_t i = 0; i < num_channels; ++i) {
            pixel[i] = i;
            if constexpr (std::is_same_v<channel_type, gil::float32_t>) {
                pixel[i] /= num_channels;
            }
        }
        return pixel;
    });

    for (std::size_t channel_index = 0; channel_index < num_channels; ++channel_index) {
        auto result = flash::to_matrix(view, channel_index);
        for (std::ptrdiff_t i = 0; i < view.height(); ++i) {
            for (std::ptrdiff_t j = 0; j < view.width(); ++j) {
                REQUIRE(result(j, i) == view(i, j)[channel_index]);
            }
        }
    }

    REQUIRE_THROWS(flash::to_matrix(view, num_channels));
    REQUIRE_THROWS(flash::to_matrix(view, num_channels + 1));
}

template <typename ImageType,
          typename OutputType = typename gil::channel_type<typename ImageType::value_type>::type>
void test_to_matrix_out()
{
    using pixel_type = typename ImageType::value_type;
    using channel_type = typename gil::channel_type<pixel_type>::type;
    constexpr auto num_channels = gil::num_channels<ImageType>{};

    pixel_type zero_pixel{};
    for (std::size_t i = 0; i < num_channels; ++i) {
        zero_pixel[i] = 0;
    }

    ImageType zero_image(16, 16, zero_pixel);
    auto view = gil::view(zero_image);

    for (std::size_t i = 0; i < num_channels; ++i) {
        blaze::DynamicMatrix<OutputType> result(view.height(), view.width());
        flash::to_matrix(view, result, i);
        REQUIRE(blaze::isZero(result));
    }

    gil::transform_pixels(view, view, [num_channels](auto) {
        pixel_type pixel;
        for (std::size_t i = 0; i < num_channels; ++i) {
            pixel[i] = i;
            if constexpr (std::is_same_v<channel_type, gil::float32_t>) {
                pixel[i] /= num_channels;
            }
        }
        return pixel;
    });

    for (std::size_t channel_index = 0; channel_index < num_channels; ++channel_index) {
        blaze::DynamicMatrix<OutputType> result(view.height(), view.width());
        flash::to_matrix(view, result, channel_index);
        for (std::ptrdiff_t i = 0; i < view.height(); ++i) {
            for (std::ptrdiff_t j = 0; j < view.width(); ++j) {
                REQUIRE(result(j, i) == view(i, j)[channel_index]);
            }
        }
    }

    blaze::DynamicMatrix<OutputType> result(view.height(), view.width());
    REQUIRE_THROWS(flash::to_matrix(view, result, num_channels));
    REQUIRE_THROWS(flash::to_matrix(view, result, num_channels + 1));
}

TEST_CASE("gray8 to_matrix", "[to_matrix]")
{
    using image_type = gil::gray8_image_t;
    test_to_matrix_type<image_type>();
    test_to_matrix_out<image_type>();
}

TEST_CASE("rgb8 to_matrix", "[to_matrix]")
{
    using image_type = gil::rgb8_image_t;
    test_to_matrix_type<image_type>();
    test_to_matrix_out<image_type>();
}

TEST_CASE("rgb32f to_matrix", "[to_matrix]")
{
    using image_type = gil::rgb32f_image_t;
    test_to_matrix_type<image_type>();
    test_to_matrix_out<image_type, float>();
}

TEST_CASE("rgb16 to_matrix", "[to_matrix]")
{
    using image_type = gil::rgb16_image_t;
    test_to_matrix_type<image_type>();
    test_to_matrix_out<image_type>();
}

TEST_CASE("rgb8s to_matrix", "[to_matrix]")
{
    using image_type = gil::rgb8s_image_t;
    test_to_matrix_type<image_type>();
    test_to_matrix_out<image_type>();
}