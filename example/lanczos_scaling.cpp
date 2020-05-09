#include <blaze/Blaze.h>
#include <flash/core.hpp>
#include <flash/numeric.hpp>
#include <flash/scaling.hpp>

#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/image.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/typedefs.hpp>

#include <CLI11.hpp>

#include <cstddef>
#include <string>

namespace gil = boost::gil;

int main(int argc, char* argv[])
{
    std::string input_file;
    std::string output_file;
    std::size_t new_width;
    std::size_t new_height;
    flash::signed_size a;

    CLI::App app{"Lanczos scaling demo"};
    app.add_option("i,--input", input_file, "PNG Grayscale input file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("o,--output",
                   output_file,
                   "PNG file that will contain grayscale output")
        ->required();
    app.add_option("w,--width", new_width, "New width to scale the source to")
        ->required();
    app.add_option("h,--height", new_height, "New height to scale source to")
        ->required();
    app.add_option(
           "a", a, "window size, the side of a window will be 2 * a + 1")
        ->required();

    CLI11_PARSE(app, argc, argv);

    gil::gray8_image_t input;
    gil::read_image(input_file, input, gil::png_tag{});

    blaze::DynamicMatrix<unsigned char> image =
        flash::to_matrix(gil::view(input));
    auto resized =
        flash::scale(flash::lanczos_method{}, image, new_width, new_height, 3);
    std::cout << resized.rows() << ' ' << resized.columns() << '\n';
    auto resized_image = flash::to_gray8_image(resized);
    gil::write_view(output_file, gil::view(resized_image), gil::png_tag{});
}