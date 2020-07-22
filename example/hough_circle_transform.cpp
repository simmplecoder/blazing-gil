#include <blaze/Blaze.h>
#include <cmath>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

int main()
{
    constexpr std::size_t size = 64;
    blaze::DynamicMatrix<int> input(size, size);

    constexpr std::size_t y0 = 32;
    constexpr std::size_t x0 = 32;
    constexpr std::size_t radius = 16;

    for (std::size_t x = x0 - radius; x < x0 + radius; ++x) {
        double c = x * x - 2 * x * x0 + x0 * x0 + y0 * y0 - radius * radius;
        double discriminant = 4 * y0 * y0 - 4 * c;
        std::size_t half_d_root = std::round(std::sqrt(discriminant) / 2);
        std::size_t y1 = y0 - half_d_root;
        std::size_t y2 = y0 + half_d_root;
        spdlog::info("x={}, y1={}, y2={}\n", x, y1, y2);

        input(y1, x) = 1;
        input(y2, x) = 1;
    }

    for (std::ptrdiff_t i = size - 1; i >= 0; --i) {
        for (std::ptrdiff_t j = 0; j < static_cast<std::ptrdiff_t>(size); ++j) {
            fmt::print("{} ", input(i, j));
        }
        fmt::print("\n");
    }
}