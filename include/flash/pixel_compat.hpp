#include <blaze/Blaze.h>
#include <blaze/math/TransposeFlag.h>
#include <boost/gil/pixel.hpp>
#include <functional>
#include <type_traits>

template <typename ChannelValue, typename Layout, bool IsRowVector = blaze::rowVector>
struct pixel_vector : boost::gil::pixel<ChannelValue, Layout> {
    using parent_t = boost::gil::pixel<ChannelValue, Layout>;
    using boost::gil::pixel<ChannelValue, Layout>::pixel;

    // TODO: rule of 5
    template <typename OtherChannelValue>
    pixel_vector(pixel_vector<OtherChannelValue, Layout> other) : parent_t(other)
    {
    }

    template <typename OtherChannelValue>
    pixel_vector& operator=(const pixel_vector<OtherChannelValue, Layout> other)
    {
        parent_t::operator=(other);
        return *this;
    }

  private:
    template <typename OtherChannelValue, typename Op>
    void perform_op(pixel_vector<OtherChannelValue, Layout> p, Op op)
    {
        constexpr auto num_channels = boost::gil::num_channels<parent_t>{};
        auto& current = *this;
        for (std::ptrdiff_t i = 0; i < num_channels; ++i) {
            current[i] = op(current[i], p[i]);
        }
    }

  public:
    // TODO: Add SFINAE for cases when parent_t::is_mutable is false
    template <typename OtherChannelValue, typename OtherLayout>
    pixel_vector& operator+=(pixel_vector<OtherChannelValue, OtherLayout> other)
    {
        perform_op(other, std::plus<>{});
        return *this;
    }

    template <typename OtherChannelValue, typename OtherLayout>
    pixel_vector& operator-=(pixel_vector<OtherChannelValue, OtherLayout> other)
    {
        perform_op(other, std::minus<>{});
        return *this;
    }

    template <typename OtherChannelValue, typename OtherLayout>
    pixel_vector& operator*=(pixel_vector<OtherChannelValue, OtherLayout> other)
    {
        perform_op(other, std::multiplies<>{});
        return *this;
    }

    template <typename OtherChannelValue, typename OtherLayout>
    pixel_vector& operator/=(pixel_vector<OtherChannelValue, OtherLayout> other)
    {
        perform_op(other, std::divides<>{});
        return *this;
    }
};

template <typename ChannelValue1, typename ChannelValue2, typename Layout, bool IsRowVector>
inline pixel_vector<std::common_type_t<ChannelValue1, ChannelValue2>, Layout>
operator+(pixel_vector<ChannelValue1, Layout, IsRowVector> lhs,
          pixel_vector<ChannelValue2, Layout, IsRowVector> rhs)
{
    lhs += rhs;
    return lhs;
}

template <typename ChannelValue1, typename ChannelValue2, typename Layout, bool IsRowVector>
inline pixel_vector<std::common_type_t<ChannelValue1, ChannelValue2>, Layout>
operator-(pixel_vector<ChannelValue1, Layout, IsRowVector> lhs,
          pixel_vector<ChannelValue2, Layout, IsRowVector> rhs)
{
    lhs -= rhs;
    return lhs;
}

template <typename ChannelValue1, typename ChannelValue2, typename Layout, bool IsRowVector>
inline pixel_vector<std::common_type_t<ChannelValue1, ChannelValue2>, Layout>
operator*(pixel_vector<ChannelValue1, Layout, IsRowVector> lhs,
          pixel_vector<ChannelValue2, Layout, IsRowVector> rhs)
{
    lhs *= rhs;
    return lhs;
}

template <typename ChannelValue1, typename ChannelValue2, typename Layout, bool IsRowVector>
inline pixel_vector<std::common_type_t<ChannelValue1, ChannelValue2>, Layout>
operator/(pixel_vector<ChannelValue1, Layout, IsRowVector> lhs,
          pixel_vector<ChannelValue2, Layout, IsRowVector> rhs)
{
    lhs /= rhs;
    return lhs;
}

template <typename Pixel, bool IsRowVector = blaze::rowVector>
struct pixel_vector_type;

template <typename ChannelValue, typename Layout, bool IsRowVector>
struct pixel_vector_type<boost::gil::pixel<ChannelValue, Layout>, IsRowVector> {
    using type = pixel_vector<ChannelValue, Layout, IsRowVector>;
};

namespace blaze
{
template <typename ChannelValue, typename Layout, bool IsRowVector>
struct UnderlyingElement<pixel_vector<ChannelValue, Layout, IsRowVector>> {
    using type = ChannelValue;
};

template <typename ChannelValue, typename Layout, bool IsRowVector>
struct IsDenseVector<pixel_vector<ChannelValue, Layout, IsRowVector>> : std::true_type {
};
} // namespace blaze