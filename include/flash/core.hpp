#pragma once

#include <algorithm>
#include <blaze/Blaze.h>
#include <blaze/math/AlignmentFlag.h>
#include <blaze/math/PaddingFlag.h>
#include <blaze/math/StorageOrder.h>
#include <blaze/math/dense/StaticVector.h>
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
#include <stdexcept>
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

/** \brief Used to bypass scoped_channel_type of GIL

    Some channel types in GIL, like `gil::float32_t` and `gil::float64_t` have a different type
    compared to it's underlying float type (because they constrain the range to be [0...1]),
    this struct will strip that and provide typedef equal to the underlying channel type.
*/
template <typename ChannelType>
struct true_channel_type {
    using type = ChannelType;
};

/// \cond specializations
template <>
struct true_channel_type<boost::gil::float32_t> {
    using type = float;
};

template <>
struct true_channel_type<boost::gil::float64_t> {
    using type = double;
};
/// \endcond

template <typename ChannelType>
using true_channel_type_t = typename true_channel_type<ChannelType>::type;

/** \brief Extract `channel`th value from each pixel in `view` and writes into `result`

   The function can be used with multi channel views, just the `channel` argument might need to be
   adjusted to select appropriate channel. Do note that it will strip wrapper type around floating
   point types like gil::float32_t, etc.

    \arg channel The channel to extract from pixels, defaults to 0
    \tparam View The source view type to convert into matrix
    \arg view The source view to convert into matrix
    \tparam MT The concrete type of matrix
    \arg result The out argument to write result into
    \tparam SO Storage order flag
*/
template <typename View, typename MT, bool SO>
void to_matrix(View view, blaze::DenseMatrix<MT, SO>& result, std::size_t channel = 0)
{
    constexpr auto num_channels = boost::gil::num_channels<View>{};
    if (channel >= num_channels) {
        throw std::invalid_argument("channel index exceeds available channels in the view");
    }
    (~result) = blaze::generate(
        view.height(), view.width(), [&view, channel](std::size_t i, std::size_t j) {
            using element_type = blaze::UnderlyingElement_t<MT>;
            return static_cast<element_type>(view(j, i)[channel]);
        });
}

/** \brief Converts an image view into `DynamicMatrix<ChannelType>`, where each entry corresponds to
   selected channel value of the pixel entry

   The function can be used with multi channel views, just the `channel` argument might need to be
   adjusted to select appropriate channel. Do note that it will strip wrapper type around floating
   point types like gil::float32_t, etc.

    \arg channel The channel to extract from pixels, defaults to 0
    \tparam View The source view type to convert into matrix
    \arg view The source view to convert into matrix
*/
template <typename View>
auto to_matrix(View view, std::size_t channel = 0)
{
    constexpr auto num_channels = boost::gil::num_channels<View>{};
    if (channel >= num_channels) {
        throw std::invalid_argument("channel index exceeds available channels in the view");
    }
    using channel_type = typename boost::gil::channel_type<View>::type;
    blaze::DynamicMatrix<true_channel_type_t<channel_type>> result(view.height(), view.width());
    to_matrix(view, result, channel);
    return result;
}

namespace detail
{
template <typename PixelType, std::size_t... indices>
auto pixel_to_vector_impl(const PixelType& pixel, std::integer_sequence<std::size_t, indices...>)
{
    using channel_t = typename boost::gil::channel_type<PixelType>::type;
    return blaze::StaticVector<true_channel_type_t<channel_t>, sizeof...(indices)>{
        pixel[indices]...};
}
} // namespace detail

/** \brief Converts a pixel into `StaticVector`

    Useful when working with multi-channel images

    \tparam PixelType The source pixel type
    \arg pixel The source pixel to convert into `StaticVector`

    \return a `StaticVector` where elements correspond to channel values, in the same order
*/
template <typename PixelType>
auto pixel_to_vector(const PixelType& pixel)
{
    return detail::pixel_to_vector_impl(
        pixel, std::make_index_sequence<boost::gil::num_channels<PixelType>{}>{});
}

/** \brief Convert possibly multi-channel view into `DynamicMatrix` of `StaticVector`s

    Works as if applying `pixel_to_vector` on each pixel to generate corresponding matrix entry

    \tparam View The source view type to convert into `DynamicMatrix`
    \arg view The source view

    \return a `DynamicMatrix<StaticVector<ChannelType, num_channel>>`, where each element
   corresponds to pixel in the view with `pixel_to_vector` applied.
*/
template <typename View>
auto to_matrix_channeled(View view)
{
    return blaze::evaluate(
        blaze::generate(view.height(), view.width(), [&view](std::size_t i, std::size_t j) {
            return pixel_to_vector(view(j, i));
        }));
}

/** \brief constructs `blaze::CustomMatrix` out of `image_view`

    Creates a matrix which is semantically like pointer, e.g. changes in the matrix
    will be reflected inside the view, and vice versa. Do note that assigning to another
    `blaze::CustomMatrix` will behave like shallow copy, thus it is important to perform
    deep copy using e.g. `blaze::DynamicMatrix` to have independent copy.

    The input view must have single channel pixels like `boost::gil::gray8_view_t`,
    for multi-channel matrices please first check layout compatibility of the pixel type
    and corresponding `blaze::StaticVector<ChannelType, num_channels>`, then use
    `as_matrix_channeled`.

    \tparam IsAligned A flag that indicates if the source view is aligned
    \tparam IsPAdded A flag that indicates if the source view is padded
    \tparam StorageOrder rowMajor or columnMajor
    \tparam SingleChannelView The type of the view to get matrix view from, must have single channel
        pixels
    \arg source The source image view to get matrix view from

    \return A `CustomMatrix<ChannelType>` with all the alignment, padding and storage order flags
*/
template <blaze::AlignmentFlag IsAligned = blaze::unaligned,
          blaze::PaddingFlag IsPadded = blaze::unpadded, bool StorageOrder = blaze::rowMajor,
          typename SingleChannelView>
auto as_matrix(SingleChannelView source)
{
    using channel_t = typename boost::gil::channel_type<SingleChannelView>::type;
    return blaze::CustomMatrix<channel_t, IsAligned, IsPadded, StorageOrder>(
        reinterpret_cast<true_channel_type_t<channel_t>*>(&source(0, 0)),
        source.height(),
        source.width());
}

/** \brief constructs `blaze::CustomMatrix` out of `image_view` whose elements are
    `StaticVector<ChannelType, num_channels>`

    Please note that there are layout incompatibilities between Blaze's `StaticVector` and
    GIL's pixel types. GIL's pixel types are padded to 4 byte boundary if the size is less than
    8 bytes, while Blaze pads to 16. There is a `static_assert` that prevents obviously wrong
    use cases, but it is advised to first check if the pixel type and resulting `StaticVector`
    are compatible, then using the function. For already tested types, please run tests for
    this function (tagged with the function name).

    \tparam IsPixelAligned Flag that indicates if individual pixels inside view are aligned.

    It seems like Blaze automatically pads if the flag is set, so do not set if padding will
    be off too.

    \tparam IsPixelPadded Flag that indicates if individual pixels inside view are padded.

    This flag is probably the source of problems. Be careful when setting it and always
    check if resulting `StaticVector` and pixel type are compatible

    \tparam IsAligned Flag that indicates if image is aligned in memory
    \tparam IsPadded Flag that indicates if individual rows are padded
    \tparam StorageOrder either rowMajor or columnMajor
    \tparam ImageView The type of image view to get channeled matrix view from
    \arg source The image view to get channeled matrix view from

    \return A `CustomMatrix<StaticVector<ChannelType, num_channel>>` with all the alignment, padding
   and storage order flags
*/
template <blaze::AlignmentFlag IsPixelAligned = blaze::unaligned,
          blaze::PaddingFlag IsPixelPadded = blaze::unpadded,
          blaze::AlignmentFlag IsAligned = blaze::unaligned,
          blaze::PaddingFlag IsPadded = blaze::unpadded, bool StorageOrder = blaze::rowMajor,
          typename ImageView>
auto as_matrix_channeled(ImageView source)
{
    using pixel_t = typename ImageView::value_type;
    using channel_t = typename boost::gil::channel_type<ImageView>::type;
    constexpr auto num_channels = boost::gil::num_channels<ImageView>{};
    using element_type = blaze::StaticVector<true_channel_type_t<channel_t>,
                                             num_channels,
                                             blaze::rowMajor,
                                             IsPixelAligned,
                                             IsPixelPadded>;
    static_assert(sizeof(pixel_t) == sizeof(element_type),
                  "The function is made to believe that pixel and corresponding vector types are"
                  "layout compatible, but they are not");
    return blaze::CustomMatrix<element_type, IsAligned, IsPadded, StorageOrder>(
        reinterpret_cast<element_type*>(&source(0, 0)), source.height(), source.width());
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

    \tparam U The type of the resulting matrix
    \arg dst_min The minimum of the resulting range, do set manually if remapping into wider range
    \arg dst_max The maximum of the resulting range, do set manually if remapping into wider range
    \tparam MT The concrete type of source matrix
    \arg source The source range to perform remapping on
    \tparam SO Storage order of the source matrix

    \return A `DynamicMatrix<StaticVector<U, length>>` where `length` if the length of vector in
   `source`
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
    auto matrix_view = as_matrix(boost::gil::view(result));
    matrix_view = source;
    return result;
}

/** \brief Converts vector into pixel

    Useful when working with multi-channel images.

    \tparam PixelType The pixel type to convert to
    \tparam VT The concrete vector type
    \arg vector The source vector to convert into `PixelType`
    \tparam TransposeFlag Transpose flag for the vector

    \return A pixel where each channel corresponds to entry in the vector, in the same order
*/
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
