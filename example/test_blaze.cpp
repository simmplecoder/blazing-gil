#include <blaze/Blaze.h>
#include <type_traits>

template <typename T>
struct identify_type;

int main() {
    auto mat = blaze::evaluate(blaze::generate(10, 10, [](std::size_t, std::size_t) {
        return blaze::StaticVector<double, 8>{1, 1, 1, 1, 1, 1, 1};
    }));
    int counter = 0;
    auto mapped = blaze::map(mat, [&counter](auto element) {
        ++counter;
        return element * counter;
    });
}