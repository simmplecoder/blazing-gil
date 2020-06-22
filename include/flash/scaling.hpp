#ifndef BLAZING_GIL_SCALING_HPP
#define BLAZING_GIL_SCALING_HPP

#include <blaze/Blaze.h>

#include <flash/core.hpp>

namespace flash
{
struct scaling_method {
};
struct nearest_neighbor : scaling_method {
};
struct bilinear_interpolation : scaling_method {
};
struct lanczos_method {
};

namespace detail
{
template <typename T>
blaze::DynamicMatrix<T>
scale_nearest_neighbor(const blaze::DynamicMatrix<T>& source,
                       std::size_t new_width, std::size_t new_height)
{
    double ratio_w = source.columns() / static_cast<double>(new_width);
    double ratio_h = source.rows() / static_cast<double>(new_height);

    return blaze::generate(
        new_height,
        new_width,
        [&source, ratio_w, ratio_h](std::size_t i, std::size_t j) {
            return source(i * ratio_h, j * ratio_w);
        });
}

template <typename T>
blaze::DynamicMatrix<T>
scale_bilinear_interpolation(const blaze::DynamicMatrix<T>& source,
                             std::size_t new_width, std::size_t new_height)
{
    auto padded = flash::pad(source, 1, 0);
    auto ratio_w = source.columns() / static_cast<double>(new_width);
    auto ratio_h = source.rows() / static_cast<double>(new_height);

    return blaze::generate(
        new_height,
        new_width,
        [&source, ratio_w, ratio_h](std::size_t i, std::size_t j) {
            // add 1 to adjust for padding
            std::size_t original_i = i * ratio_h + 1;
            std::size_t original_j = j * ratio_w + 1;

            auto i_diff = (i * ratio_h) - original_i;
            auto j_diff = (j * ratio_w) - original_j;
            auto a = source(original_i, original_j);
            auto b = source(original_i, original_j + 1);
            auto c = source(original_i + 1, original_j);
            auto d = source(original_i + 1, original_j);
            return a * (1 - j_diff) * (1 - i_diff) + b * j_diff * (1 - i_diff) +
                   c * (1 - j_diff) * i_diff + d * j_diff * i_diff;
        });
}
} // namespace detail

template <typename Method, typename T>
blaze::DynamicMatrix<T> scale(Method, const blaze::DynamicMatrix<T>& source,
                              std::size_t new_width, std::size_t new_height);

template <typename T>
blaze::DynamicMatrix<T> scale(nearest_neighbor,
                              const blaze::DynamicMatrix<T>& source,
                              std::size_t new_width, std::size_t new_height)
{
    return detail::scale_nearest_neighbor(source, new_width, new_height);
}

template <typename T>
blaze::DynamicMatrix<T> scale(bilinear_interpolation,
                              const blaze::DynamicMatrix<T>& source,
                              std::size_t new_width, std::size_t new_height)
{
    return detail::scale_bilinear_interpolation(source, new_width, new_height);
}

inline double normalized_sinc(double x) { return std::sin(x * pi) / (x * pi); }

inline double lanczos(double x, signed_size a)
{
    if (x <= 0 && 0 <= x)
        return 1;

    if (-a < x && x < a)
        return normalized_sinc(x) / normalized_sinc(x / static_cast<double>(a));
    return 0;
}

template <typename T>
blaze::DynamicMatrix<T>
scale(lanczos_method, const blaze::DynamicMatrix<T>& source,
      std::size_t new_width, std::size_t new_height, signed_size a)
{
    double ratio_w = source.columns() / static_cast<double>(new_width);
    double ratio_h = source.rows() / static_cast<double>(new_height);
    return blaze::generate(
        new_height,
        new_width,
        [a, ratio_w, ratio_h, &source](std::size_t target_i,
                                       std::size_t target_j) {
            signed_size original_i = target_i * ratio_h;
            signed_size original_j = target_j * ratio_w;
            T result{};
            for (signed_size i = original_i - a; i <= original_i + a; ++i) {
                for (signed_size j = original_j - a; j <= original_j + a; ++j) {
                    if (i < 0 || i >= static_cast<signed_size>(source.rows()))
                        continue;
                    if (j < 0 ||
                        j >= static_cast<signed_size>(source.columns()))
                        continue;
                    auto lanczos_response =
                        lanczos(original_i - i, a) * lanczos(original_j - j, a);
                    result = result +
                             lanczos_response * source(original_i, original_j);
                }
            }
            return result;
        });
}
} // namespace flash

#endif