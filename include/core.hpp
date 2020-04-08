#include <blaze/math/AlignmentFlag.h>
#include <blaze/math/PaddingFlag.h>
#include <blaze/math/CustomMatrix.h>
#include <blaze/math/dense/CustomMatrix.h>

#include <boost/gil/image_view.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/typedefs.hpp>
#include <limits>
#include <type_traits>

namespace flash {

template <typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T>
using image_matrix = blaze::CustomMatrix<T, blaze::unaligned, blaze::unpadded>;

template <typename GrayView>
auto to_matrix(GrayView source)
{
    using value_type = remove_cvref_t<decltype(std::declval<typename GrayView::value_type>().at(std::integral_constant<int, 0>{}))>;
    return image_matrix<value_type>(reinterpret_cast<value_type*>(&source(0, 0)), 
                                       source.height(), 
                                       source.width());
}

// perform linear mapping from source range to destination range
// only use with blaze::min and blaze::max 
// note that this is intended to narrow from source range
// to destination range
template <typename U, typename SourceMatrix, typename T>
auto remap_to(const SourceMatrix& source, T src_min, T src_max, U dst_min = std::numeric_limits<U>::min(), U dst_max = std::numeric_limits<U>::max())
{
    // ensure that dst_max - dst_min will not overflow
    return blaze::map(source, [src_min, dst_min, src_length = src_max - src_min, dst_length = static_cast<T>(dst_max) - dst_min](T value) {
        // attempt to avoid integral inaccuracy
        return static_cast<U>(dst_min + static_cast<double>(value - src_min) / src_length * dst_length);
    });
}

template <typename U, typename SourceMatrix>
auto remap_to(const SourceMatrix& source)
{
    return remap_to<U>(source, blaze::min(source), blaze::max(source));
}

boost::gil::gray8_image_t to_gray8_image(const blaze::DynamicMatrix<std::uint8_t>& source)
{
    boost::gil::gray8_image_t result(source.columns(), source.rows());
    auto matrix_view = to_matrix(boost::gil::view(result));
    matrix_view = source;
    return result;
}
}
