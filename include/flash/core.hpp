#pragma once

#include <algorithm>
#include <blaze/Blaze.h>
#include <blaze/math/expressions/Forward.h>
#include <blaze/math/typetraits/IsDenseVector.h>
#include <blaze/math/typetraits/IsStatic.h>
#include <blaze/math/typetraits/UnderlyingElement.h>
#include <boost/gil/algorithm.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/pixel.hpp>
#include <boost/gil/typedefs.hpp>
#include <iostream>
#include <limits>
#include <type_traits>
#include <utility>

namespace std
{
template <typename U, std::size_t N>
struct numeric_limits<blaze::StaticVector<U, N>> {
    static constexpr auto min()
    {
        auto v = blaze::StaticVector<U, N>{};
        std::fill(v.begin(), v.end(), std::numeric_limits<U>::min());

        return v;
    }

    static constexpr auto max()
    {
        auto v = blaze::StaticVector<U, N>{};
        std::fill(v.begin(), v.end(), std::numeric_limits<U>::max());

        return v;
    }
};

template <typename T, typename U, std::size_t N>
struct common_type<blaze::StaticVector<T, N>, blaze::StaticVector<U, N>> {
    using type = blaze::StaticVector<std::common_type<T, U>, N>;
};

} // namespace std

/** \brief the library namespace - flash

    Contains all functions and classes in the library
*/
namespace flash
{

using signed_size = std::ptrdiff_t;
inline constexpr double pi = 3.14159265358979323846;

template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T>
using image_matrix = blaze::CustomMatrix<T, blaze::unaligned, blaze::unpadded>;

namespace detail
{
template <typename PixelType, std::size_t... indices>
auto pixel_to_vector_impl(const PixelType& pixel, std::integer_sequence<std::size_t, indices...>)
{
    using ChannelType = typename boost::gil::channel_type<PixelType>::type;
    return blaze::StaticVector<ChannelType, sizeof...(indices)>{pixel[indices]...};
}
} // namespace detail

template <typename PixelType>
auto pixel_to_vector(const PixelType& pixel)
{
    return detail::pixel_to_vector_impl(
        pixel, std::make_index_sequence<boost::gil::num_channels<PixelType>{}>{});
}

template <typename View>
auto to_matrix_channeled(View view)
{
    return blaze::evaluate(
        blaze::generate(view.height(), view.width(), [&view](std::size_t i, std::size_t j) {
            return pixel_to_vector(view(j, i));
        }));
}

template <typename GrayView>
auto to_matrix(GrayView source)
{
    using value_type = remove_cvref_t<decltype(
        std::declval<typename GrayView::value_type>().at(std::integral_constant<int, 0>{}))>;
    return image_matrix<value_type>(
        reinterpret_cast<value_type*>(&source(0, 0)), source.height(), source.width());
}

// perform linear mapping from source range to destination range
// only use with blaze::min and blaze::max
// note that this is intended to narrow from source range
// to destination range
template <typename U, typename SourceMatrix, typename T>
auto remap_to(const SourceMatrix& source, T src_min, T src_max,
              U dst_min = std::numeric_limits<U>::min(), U dst_max = std::numeric_limits<U>::max())
{
    // ensure that dst_max - dst_min will not overflow
    return blaze::map(source,
                      [src_min,
                       dst_min,
                       src_length = src_max - src_min,
                       dst_length = static_cast<T>(dst_max) - dst_min](T value) {
                          // attempt to avoid integral inaccuracy
                          return static_cast<U>(dst_min + static_cast<double>(value - src_min) /
                                                              src_length * dst_length);
                      });
}

template <typename U, typename SourceMatrix>
auto remap_to(const SourceMatrix& source)
{
    return remap_to<U>(source, blaze::min(source), blaze::max(source));
}

/** \brief Remap channeled matrix into another range

    The class takes minimum and maximum along each channel and remaps it into new range. Do note
    that if a wider type is specified, `dst_min` and `dst_max` vectors has to be manually specified
    and different than type min/max. Otherwise, overflow is guaranteed.
*/
template <typename U, typename MT, bool SO>
auto remap_to_channeled(const blaze::DenseMatrix<MT, SO>& source,
                        U dst_min = std::numeric_limits<U>::min(),
                        U dst_max = std::numeric_limits<U>::max())
{
    using source_vector_type = blaze::UnderlyingElement_t<MT>;
    constexpr auto vector_size = source_vector_type::size();
    using element_type = blaze::UnderlyingElement_t<source_vector_type>;
    static_assert(blaze::IsStatic_v<source_vector_type> &&
                  blaze::IsDenseVector_v<source_vector_type>);
    auto min_reducer = [](source_vector_type prev, const source_vector_type& current) {
        for (std::size_t i = 0; i < prev.size(); ++i) {
            if (prev[i] > current[i]) {
                prev[i] = current[i];
            }
        }

        return prev;
    };
    auto src_min_elems = blaze::reduce(source, min_reducer);

    auto max_reducer = [](source_vector_type prev, const source_vector_type& current) {
        for (std::size_t i = 0; i < prev.size(); ++i) {
            if (prev[i] < current[i]) {
                prev[i] = current[i];
            }
        }

        return prev;
    };
    auto src_max_elems = blaze::reduce(source, max_reducer);

    using result_vector_type = blaze::StaticVector<U, source_vector_type::size()>;

    // pick according to std::common_type rules to make
    // more resistant to overflow/underflow and accuracy loss
    using proxy_type = std::common_type_t<element_type, U>;
    using proxy_vector = blaze::StaticVector<proxy_type, source_vector_type::size()>;

    result_vector_type dst_min_elems;
    std::fill(dst_min_elems.begin(), dst_min_elems.end(), dst_min);
    result_vector_type dst_max_elems;
    std::fill(dst_max_elems.begin(), dst_max_elems.end(), dst_max);

    proxy_vector src_range_length = src_max_elems - src_min_elems + 1;
    proxy_vector dst_range_length = dst_max_elems - dst_min_elems + 1;

    return blaze::evaluate(blaze::map(
        (~source),
        [src_min_elems, dst_min_elems, src_range_length, dst_range_length, vector_size](
            const source_vector_type& elem) {
            result_vector_type result{};
            for (std::size_t i = 0; i < vector_size; ++i) {
                result[i] = dst_min_elems[i] +
                            (static_cast<double>(elem[i] -
                                                 src_min_elems[i]) / // cast to double to make more
                                                                     // resistant to accuracy loss
                             src_range_length[i] *
                             dst_range_length[i]);
            }

            return result;
        }));
}

inline boost::gil::gray8_image_t to_gray8_image(const blaze::DynamicMatrix<std::uint8_t>& source)
{
    boost::gil::gray8_image_t result(source.columns(), source.rows());
    auto matrix_view = to_matrix(boost::gil::view(result));
    matrix_view = source;
    return result;
}

template <typename PixelType, typename VT, bool TransposeFlag>
auto vector_to_pixel(const blaze::DenseVector<VT, TransposeFlag>& vector)
{
    auto num_channels = boost::gil::num_channels<PixelType>{};
    PixelType pixel{};
    for (std::size_t i = 0; i < num_channels; ++i) {
        pixel[i] = (~vector)[i];
    }

    return pixel;
}

template <typename ImageType, typename PixelType = typename ImageType::value_type, typename MT>
ImageType to_image(const blaze::DenseMatrix<MT, blaze::rowMajor>& data)
{
    ImageType image((~data).rows(), (~data).columns());
    auto view = boost::gil::view(image);

    for (signed_size i = 0; i < view.height(); ++i) {
        for (signed_size j = 0; j < view.width(); ++j) {
            view(j, i) = vector_to_pixel<PixelType>((~data)(i, j));
        }
    }

    return image;
}

template <typename T, typename U>
blaze::DynamicMatrix<T> pad(const blaze::DynamicMatrix<T>& source, std::size_t pad_count,
                            const U& padding_value)
{
    static_assert(std::is_convertible_v<T, U>);

    blaze::DynamicMatrix<T> result(source.rows() + pad_count * 2, source.columns() + pad_count * 2);

    auto full_resulting_width = source.columns() + pad_count * 2;
    // first pad_count rows
    blaze::submatrix(result, 0, 0, pad_count, full_resulting_width) = padding_value;
    // last pad_count rows
    blaze::submatrix(result, source.rows(), 0, pad_count, full_resulting_width) = padding_value;

    auto vertical_block_height = source.rows();
    // left pad_count columns, do note that top pad_count rows are already
    // filled
    blaze::submatrix(result, pad_count, 0, vertical_block_height, pad_count) = padding_value;
    // right pad_count columns, do note that top pad_count rows are already
    // filled
    blaze::submatrix(result, pad_count, source.columns(), vertical_block_height, pad_count) =
        padding_value;

    // don't forget to copy the original contents
    blaze::submatrix(result, pad_count, pad_count, source.rows(), source.columns()) = source;

    return result;
}
} // namespace flash
