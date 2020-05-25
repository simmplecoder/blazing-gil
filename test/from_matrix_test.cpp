#include <boost/gil/typedefs.hpp>
#include <catch2/catch.hpp>
#include <cstdint>
#include <flash/core.hpp>
#include <type_traits>

namespace gil = boost::gil;

template <typename PixelType>
PixelType create_pixel(std::uint8_t value)
{
    PixelType pixel;
    auto num_channels = gil::num_channels<PixelType>::value;
    for (flash::signed_size i = 0; i < num_channels; ++i) {
        pixel[i] = value;
    }

    return pixel;
}

template <typename PixelType>
PixelType create_zero_pixel()
{
    return create_pixel<PixelType>(0);
}

template <typename VectorType, typename ImageType,
          typename PixelType = typename ImageType::value_type>
void test_vector_matrix_type()
{
    blaze::DynamicMatrix<VectorType> input(16, 16, VectorType({0}));
    auto result = flash::from_matrix<ImageType>(input);
    STATIC_REQUIRE(std::is_same_v<decltype(result), ImageType>);

    ImageType expected(16, 16, create_zero_pixel<PixelType>());

    REQUIRE(result == expected);

    std::uint8_t value = 23;
    constexpr auto num_channels = gil::num_channels<PixelType>{};
    for (flash::signed_size i = 0; i < num_channels; ++i) {
        auto pixel = create_zero_pixel<PixelType>();
        pixel[i] = value;
        ImageType expected(16, 16, pixel);

        auto vector = VectorType{0};
        vector[i] = value;
        blaze::DynamicMatrix<VectorType> input(16, 16, vector);

        auto result = flash::from_matrix<ImageType, PixelType>(input);
        REQUIRE(result == expected);
    }

    // different scope
    {
        PixelType pixel;
        VectorType vector;
        auto num_channels = gil::num_channels<PixelType>::value;
        for (flash::signed_size i = 0; i < num_channels; ++i) {
            pixel[i] = i;
            vector[static_cast<std::size_t>(i)] = i;
        }

        ImageType expected(16, 16, pixel);
        blaze::DynamicMatrix<VectorType> input(16, 16, vector);

        auto result = flash::from_matrix<ImageType, PixelType>(input);
        REQUIRE(result == expected);
    }
}

template <typename ScalarType, typename ImageType,
          typename PixelType = typename ImageType::value_type>
void test_scalar_matrix_type()
{
    blaze::DynamicMatrix<ScalarType> input(16, 16, 0);
    ImageType expected(16, 16, create_zero_pixel<PixelType>());

    auto result = flash::from_matrix<ImageType, PixelType>(input);
    STATIC_REQUIRE(std::is_same_v<decltype(result), ImageType>);
    REQUIRE(result == expected);

    // same but non-zero value
    ScalarType value = 23;
    input = value;
    gil::fill_pixels(gil::view(expected), PixelType(value));
    auto uniform_test_result = flash::from_matrix<ImageType, PixelType>(input);
    REQUIRE(uniform_test_result == expected);

    auto view = gil::view(expected);

    input(0, 0) = 0;
    input(0, 10) = 0;
    view(0, 0)[0] = 0;
    view(10, 0)[0] = 0;

    auto nonuniform_test_result = flash::from_matrix<ImageType, PixelType>(input);
    REQUIRE(nonuniform_test_result == expected);
}

TEST_CASE("Matrix to rgb8 image", "[from_matrix]")
{
    using image_type = gil::rgb8_image_t;
    using vector_type = blaze::StaticVector<gil::uint8_t, 3>;
    test_vector_matrix_type<vector_type, image_type>();
}

TEST_CASE("Matrix to rgb32f image", "[from_matrix]")
{
    using image_type = gil::rgb32f_image_t;
    using vector_type = blaze::StaticVector<float, 3>;
    test_vector_matrix_type<vector_type, image_type>();
}

TEST_CASE("Matrix to rgba32f image", "[from_matrix]")
{
    using image_type = gil::rgba32f_image_t;
    using vector_type = blaze::StaticVector<float, 4>;
    test_vector_matrix_type<vector_type, image_type>();
}

TEST_CASE("Matrix to rgba8 image", "[from_matrix]")
{
    using image_type = gil::rgba8_image_t;
    using vector_type = blaze::StaticVector<gil::uint8_t, 4>;
    test_vector_matrix_type<vector_type, image_type>();
}

TEST_CASE("Matrix to gray8 image", "[from_matrix]")
{
    using image_type = gil::gray8_image_t;
    using vector_type = blaze::StaticVector<gil::uint8_t, 1>;
    test_vector_matrix_type<vector_type, image_type>();
}

TEST_CASE("Matrix to gray16 image", "[from_matrix]")
{
    using image_type = gil::gray8_image_t;
    using vector_type = blaze::StaticVector<gil::uint16_t, 1>;
    test_vector_matrix_type<vector_type, image_type>();
}

TEST_CASE("Matrix to gray32f image", "[from_matrix]")
{
    using image_type = gil::gray32f_image_t;
    using vector_type = blaze::StaticVector<float, 1>;
    test_vector_matrix_type<vector_type, image_type>();
}

TEST_CASE("Scalar matrix to gray8 image", "[from_matrix]")
{
    using image_type = gil::gray8_image_t;
    using scalar_type = std::uint8_t;
    test_scalar_matrix_type<scalar_type, image_type>();    
}

TEST_CASE("Scalar matrix to gray16 image", "[from_matrix]")
{
    using image_type = gil::gray16_image_t;
    using scalar_type = std::uint16_t;
    test_scalar_matrix_type<scalar_type, image_type>();    
}

TEST_CASE("Scalar matrix to gray32f image", "[from_matrix]")
{
    using image_type = gil::gray32f_image_t;
    using scalar_type = float;
    test_scalar_matrix_type<scalar_type, image_type>();    
}
