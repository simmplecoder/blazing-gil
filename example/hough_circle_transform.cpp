#include <blaze/Blaze.h>
#include <cmath>
#include <flash/core.hpp>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/typedefs.hpp>

template <typename MT, typename T>
void rasterize_circle(MT& view, std::ptrdiff_t x0, std::ptrdiff_t y0, std::size_t radius,
                      T fill_pixel)
{
    auto translate_mirror_points = [&view, fill_pixel, x0, y0](std::ptrdiff_t relative_x,
                                                               std::ptrdiff_t relative_y) {
        auto assign = [&view, fill_pixel](std::ptrdiff_t x, std::ptrdiff_t y) {
            view(y, x) = fill_pixel;
        };
        auto y_pos = y0 + relative_y;
        auto y_neg = y0 - relative_y;
        auto x_pos = x0 + relative_x;
        auto x_neg = x0 - relative_x;
        assign(x_pos, y_pos);
        assign(y_pos, x_pos);
        assign(y_pos, x_neg);
        assign(x_pos, y_neg);
        assign(x_neg, y_neg);
        assign(y_neg, x_neg);
        assign(y_neg, x_pos);
        assign(x_neg, y_pos);
    };

    double _45_degrees = flash::pi / 4.0;
    std::ptrdiff_t target = std::round(radius * std::cos(_45_degrees));
    std::ptrdiff_t y_current = radius;
    double r_squared = radius * radius;
    translate_mirror_points(0, y_current);
    for (std::ptrdiff_t x = 1; x <= target; ++x) {
        auto y = y_current - 0.5;
        double midpoint = x * x + y * y - r_squared;
        if (midpoint < 0.0) {
            translate_mirror_points(x, y_current);
        } else {
            y_current -= 1;
            translate_mirror_points(x, y_current);
        }
    }
}

int main()
{
    constexpr std::size_t size = 64;
    blaze::DynamicMatrix<std::uint8_t> input(size, size);

    constexpr std::size_t y0 = 32;
    constexpr std::size_t x0 = 32;
    constexpr std::size_t radius = 16;

    // for (std::size_t x = x0 - radius; x < x0 + radius; ++x) {
    //     double c = x * x - 2 * x * x0 + x0 * x0 + y0 * y0 - radius * radius;
    //     double discriminant = 4 * y0 * y0 - 4 * c;
    //     std::size_t half_d_root = std::round(std::sqrt(discriminant) / 2);
    //     std::size_t y1 = y0 - half_d_root;
    //     std::size_t y2 = y0 + half_d_root;
    //     spdlog::info("x={}, y1={}, y2={}\n", x, y1, y2);

    //     input(y1, x) = 1;
    //     input(y2, x) = 1;
    // }
    rasterize_circle(input, x0, y0, radius, 255);
    auto image = flash::from_matrix<boost::gil::gray8_image_t>(input);
    boost::gil::write_view("circle.png", boost::gil::view(image), boost::gil::png_tag{});

    // for (std::ptrdiff_t i = size - 1; i >= 0; --i) {
    //     for (std::ptrdiff_t j = 0; j < static_cast<std::ptrdiff_t>(size); ++j) {
    //         fmt::print("{} ", input(i, j));
    //     }
    //     fmt::print("\n");
    // }
}