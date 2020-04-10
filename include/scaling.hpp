#include <blaze/math/dense/DynamicMatrix.h>
#include <blaze/math/expressions/DMatGenExpr.h>

namespace flash 
{
    struct scaling_method {};
    struct nearest_neighbor: scaling_method {};

    namespace detail
    {
        template <typename T>
        blaze::DynamicMatrix<T> scale_nearest_neighbor(const blaze::DynamicMatrix<T>& source, std::size_t new_width, std::size_t new_height)
        {
            double ratio_w = source.columns() / static_cast<double>(new_width);
            double ratio_h = source.rows() / static_cast<double>(new_height);

            return blaze::generate(new_height, new_width, [&source, ratio_w, ratio_h](std::size_t i, std::size_t j) {
                return source(i * ratio_h, j * ratio_w);
            });
        }
    }

    template <typename Method, typename T>
    blaze::DynamicMatrix<T> scale(Method, const blaze::DynamicMatrix<T>& source, std::size_t new_width, std::size_t new_height);

    template <typename T>
    blaze::DynamicMatrix<T> scale(nearest_neighbor, const blaze::DynamicMatrix<T>& source, std::size_t new_width, std::size_t new_height)
    {
        return detail::scale_nearest_neighbor(source, new_width, new_height);
    }
}