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

TEST_CASE("Matrix to rgb8 image", "[from_matrix]")
{
    using image_type = gil::rgb8_image_t;
    using vector_type = blaze::StaticVector<gil::uint8_t, 3>;
    test_vector_matrix_type<vector_type, image_type>();
}

// TEST_CASE("matrix to gray8 image conversion typecheck", "[to_matrix_channeled]")
// {
//     // gil::gray8_image_t input(16, 16, gil::gray8_pixel_t(13));
//     // auto matrix = flash::to_matrix_channeled(gil::view(input));
//     // STATIC_REQUIRE(std::is_same_v<blaze::StaticVector<std::uint8_t, 1>,
//     //                               blaze::UnderlyingElement_t<decltype(matrix)>>);
//     blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 1>> matrix(16, 16);
//     auto image = flash::from_matrix<gil::gray8_image_t>(matrix);
//     STATIC_REQUIRE(std::is_same_v<decltype(image), gil::gray8_image_t>);
// }

// TEST_CASE("matrix to gray8 image values check", "[to_matrix_channeled]")
// {
//     // gil::gray8_image_t input(16, 16, gil::gray8_pixel_t(13));
//     // auto matrix = flash::to_matrix_channeled(gil::view(input));
//     // blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 1>> expected(16, 16, {13});

//     // REQUIRE(matrix == expected);
//     const std::uint8_t value = 23;
//     blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 1>> matrix(16, 16, {value});
//     gil::gray8_image_t expected(16, 16, gil::gray8_pixel_t(value));

//     auto result = flash::from_matrix<gil::gray8_image_t>(matrix);
//     REQUIRE(result == expected);
// }

// TEST_CASE("matrix with differing values to gray8 image value check", "[to_matrix_channeled]")
// {
//     gil::gray8_image_t expected(16, 16, gil::gray8_pixel_t(13));
//     auto view = gil::view(expected);
//     view(0, 0)[0] = 0;
//     view(1, 0)[0] = 1; // rows and cols are different for GIL vs Blaze

//     blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 1>> input(16, 16, {13});
//     input(0, 0)[0] = 0;
//     input(0, 1)[0] = 1;
//     auto result = flash::from_matrix<gil::gray8_image_t>(input);

//     REQUIRE(result == expected);
// }

// TEST_CASE("matrix to rgb8 image typecheck", "[to_matrix_channeled]")
// {
//     gil::rgb8_pixel_t default_pixel(1, 2, 3);
//     gil::rgb8_image_t result(16, 16, default_pixel);

//     blaze::StaticVector<std::uint8_t, 3> default_vector({1, 2, 3});
//     blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 3>> input(16, 16, default_vector);

//     auto result = flash::from_matrix<gil::rgb8_image_t>(gil::view(input));
//     STATIC_REQUIRE(std::is_same_v<gil::rgb8_image_t, decltype(result)>);
// }

// TEST_CASE("matrix to rgb8 image value check", "[to_matrix_channeled]")
// {
//     gil::rgb8_pixel_t default_pixel(1, 2, 3);
//     gil::rgb8_image_t expected(16, 16, default_pixel);

//     blaze::StaticVector<std::uint8_t, 3> default_vector({1, 2, 3});
//     blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 3>> input(16, 16, default_vector);

//     auto result = flash::to_matrix_channeled(gil::view(input));
//     REQUIRE(result == expected);
// }

// TEST_CASE("matrix with differing values to rgb8 image value check", "[to_matrix_channeled]")
// {
//     gil::rgb8_pixel_t default_pixel(1, 2, 3);
//     gil::rgb8_image_t expected(16, 16, default_pixel);
//     auto view = gil::view(expected);
//     view(0, 0) = gil::rgb8_pixel_t(10, 20, 30);
//     view(1, 0) = gil::rgb8_pixel_t(50, 50, 50);

//     blaze::StaticVector<std::uint8_t, 3> default_vector({1, 2, 3});
//     blaze::DynamicMatrix<blaze::StaticVector<std::uint8_t, 3>> input(16, 16, default_vector);
//     expected(0, 0) = {10, 20, 30};
//     expected(0, 1) = {50, 50, 50};

//     auto result = flash::to_matrix_channeled(gil::view(input));
//     REQUIRE(result == expected);
// }