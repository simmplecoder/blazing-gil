#include <blaze/math/AlignmentFlag.h>
#include <blaze/math/PaddingFlag.h>
#include <blaze/math/CustomMatrix.h>
#include <blaze/math/dense/CustomMatrix.h>

#include <boost/gil/image_view.hpp>
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
template <typename U, typename SourceMatrix, typename T>
auto remap_to(const SourceMatrix& source, T src_min, T src_max, U dst_min = std::numeric_limits<U>::min(), U dst_max = std::numeric_limits<U>::max())
{
    return blaze::map(source, [src_min, src_max, dst_min, dst_max](T value) {
        // attempt to avoid integral inaccuracy
        return static_cast<unsigned char>(dst_min + static_cast<double>(value - src_min) / src_max * dst_max);
    });
}

template <typename U, typename SourceMatrix>
auto remap_to(const SourceMatrix& source)
{
    return remap_to<U>(source, blaze::min(source), blaze::max(source));
}
}