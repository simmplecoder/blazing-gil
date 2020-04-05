#include <blaze/math/DynamicMatrix.h>
#include <blaze/math/StaticMatrix.h>
#include <blaze/math/Submatrix.h>
#include <blaze/math/dense/StaticMatrix.h>

namespace flash {
template <typename T>
using kernel2d = blaze::DynamicMatrix<T>;

template <typename T, std::size_t M, std::size_t N>
using kernel2d_fixed = blaze::StaticMatrix<T, M, N>;

static const kernel2d_fixed<std::int16_t, 3, 3> sobel_x{{1, 0, -1}, {2, 0, -2}, {1, 0, -1}};
static const kernel2d_fixed<std::int16_t, 3, 3> sobel_y{{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};

template <typename Kernel>
Kernel flip_kernel(const Kernel& source)
{
    auto m = source.rows();
    auto n = source.columns();
    // temporary fix for diverging constructors 
    // for different matrix types
    // should be fast enough, as kernels are
    // usually small   
    Kernel result(source);
    for (std::size_t i = 0; i < m; ++i)
    {
        for (std::size_t j = 0; j < n; ++j)
        {
            result(j, i) = source(m - i - 1, n - j - 1);
        }
    }

    return result;
}

template <typename T, typename Kernel>
blaze::DynamicMatrix<T> convolve(const blaze::DynamicMatrix<T>& source, const Kernel& original_kernel)
{
    auto kernel = flip_kernel(original_kernel);
    auto m = source.rows();
    auto n = source.columns();

    blaze::DynamicMatrix<T> result(m, n, 0);
    auto kernel_size = kernel.rows();

    if (m < kernel_size || n < kernel_size)
    {
        return result;
    }

    for (std::size_t i = kernel_size; i < m - kernel_size; ++i)
    {
        for (std::size_t j = kernel_size; j < n - kernel_size; ++j)
        {
            auto current = blaze::submatrix(source, i, j, kernel_size, kernel_size);
            result(i, j) = blaze::sum(current % kernel);
        }
    }

    return result;
}
}