#pragma once

#include <numbers>

#include <linalg.h>



namespace la = linalg;

using Vec1f = la::vec<float,1>;
using Vec1i = la::vec<std::int32_t,1>;
using Vec1u = la::vec<std::uint32_t,1>;

using Vec2f = la::vec<float,2>;
using Vec2i = la::vec<std::int32_t,2>;
using Vec2u = la::vec<std::uint32_t,2>;


using Vec3f   = la::vec<float,3>;
using Vec3i   = la::vec<std::int32_t,3>;
using Vec3u   = la::vec<std::uint32_t,3>;

using Color3f = Vec3f;
using Color3u = Vec3u;


using Vec4f = la::vec<float,4>;
using Vec4i = la::vec<std::int32_t,4>;
using Vec4u = la::vec<std::uint32_t,4>;

using Color4f = Vec4f;
using Color4u = Vec4u;

using Mat22f = la::mat<float,2,2>;
using Mat22d = la::mat<double,2,2>;
using Mat33f = la::mat<float,3,3>;
using Mat33d = la::mat<double,3,3>;
using Mat44f = la::mat<float,4,4>;
using Mat44d = la::mat<double,4,4>;


using Pixel = Vec2f;
