#pragma once

#include <blaze/math/dense/DynamicMatrix.h>
#include <blaze/math/expressions/DMatGenExpr.h>

#include <core.hpp>

namespace flash 
{
    struct scaling_method {};
    struct nearest_neighbor: scaling_method {};
    struct bilinear_interpolation: scaling_method {};

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

        template <typename T>
        blaze::DynamicMatrix<T> scale_bilinear_interpolation(const blaze::DynamicMatrix<T>& source, std::size_t new_width, std::size_t new_height)
        {
            auto padded = flash::pad(source, 1, 0);
            auto ratio_w = source.columns() / static_cast<double>(new_width);
            auto ratio_h = source.rows() / static_cast<double>(new_height);

            return blaze::generate(new_height, new_width, [&source, ratio_w, ratio_h](std::size_t i, std::size_t j)
            {
                // add 1 to adjust for padding
                std::size_t original_i = i * ratio_h + 1;
                std::size_t original_j = j * ratio_w + 1;

                auto i_diff = (i * ratio_h) - original_i;
                auto j_diff = (j * ratio_w) - original_j;
                auto a = source(original_i, original_j);
                auto b = source(original_i, original_j + 1);
                auto c = source(original_i + 1, original_j);
                auto d = source(original_i + 1, original_j);
                return a * (1 - j_diff) * (1 - i_diff) + b * j_diff * (1 - i_diff)
                       + c * (1 - j_diff) * i_diff + d * j_diff * i_diff;
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

    template <typename T>
    blaze::DynamicMatrix<T> scale(bilinear_interpolation, const blaze::DynamicMatrix<T>& source, std::size_t new_width, std::size_t new_height)
    {
        return detail::scale_bilinear_interpolation(source, new_width, new_height);
    }
}