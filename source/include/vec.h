#ifndef VECTOR_H
#define VECTOR_H

#include <concepts>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <optional>
#include <random>


template<std::size_t Dim>
requires(Dim==2 || Dim==3)
class Vec {
protected:
    std::array<float,Dim> elem;
public:
    //Constructor for 2D vectors
    explicit constexpr Vec() requires(Dim==2) = default;
    explicit constexpr Vec(float x, float y) requires(Dim==2) 
        : elem{x,y} {}
    
    //Constructor for 3D vectors
    explicit constexpr Vec() requires(Dim==3) = default;
    explicit constexpr Vec(float x, float y, float z) requires(Dim==3) 
        : elem{x,y,z} {}

    constexpr float X() const noexcept {return elem[0];}
    constexpr float Y() const noexcept {return elem[1];}
    constexpr float Z() const noexcept requires(Dim==3)  {return elem[2];}

    constexpr Vec operator-()  noexcept {
        Vec<Dim> ret(*this);
        std::transform(this->cbegin(),this->cend(),
                   ret.begin(),
                   [](auto v) {return -v;});
        return ret;
    }

    constexpr float& operator[](int i) {return elem[i];}    
    constexpr const float& operator[](int i) const {return elem[i];}

    
    constexpr Vec& operator+=(const Vec& other) {
        std::transform(this->cbegin(),this->cend(), 
                       other.cbegin(), this->begin(),
                       [](float l, float r){return l+r;});
        return *this;
    }
    constexpr Vec& operator*=(const float t)  {
        std::transform(this->cbegin(),this->cend(), 
                       [t](auto e){return e*t;});
        return *this;
    }
    
    constexpr Vec& operator/=(const float t)  {
        assert(t!=0);
        return *this *= 1/t;
    }

    constexpr float LengthSquared() const noexcept {return std::inner_product(this->cbegin(),this->cend(),this->cbegin(), 0);}
    constexpr float Length() const {return sqrtf(LengthSquared());}

    const auto& Data() const {return elem;}

    //Iterators
    auto begin() { return elem.begin(); }
    auto end() { return elem.end(); }
    auto cbegin() const { return elem.begin(); }
    auto cend() const { return elem.end(); }
    auto begin() const { return elem.begin(); }
    auto end() const { return elem.end(); }

    template <std::size_t S>      // all instantiations of this template are my friends
    friend constexpr bool operator==(const Vec<S>& lhs, const Vec<S>& rhs);

};

//Aliases
using Vec2 = Vec<2>;
using Vec3 = Vec<3>;
using Color3 = Vec<3>;
using Point3 = Vec<3>;

template<std::size_t Dim>
inline constexpr bool operator == (const Vec<Dim>& lhs, const Vec<Dim>& rhs)
{
    return lhs.elem == rhs.elem;
}

template<std::size_t Dim>
inline constexpr Vec<Dim> operator+(const Vec<Dim>& lhs, const Vec<Dim>& rhs) 
{
    auto ret{lhs};
    std::transform(lhs.cbegin(),lhs.cend(),
                   rhs.cbegin(), ret.begin(),
                   [](auto l, auto r) {return l+r;}
                  );
    return ret;
}

template<std::size_t Dim>
inline constexpr Vec<Dim> operator-(const Vec<Dim>& lhs, const Vec<Dim>& rhs) 
{
    auto ret{lhs};
    std::transform(lhs.cbegin(),lhs.cend(),
                   rhs.cbegin(), ret.begin(),
                   []( auto l, auto r) {return l-r;}
                  );          
    return ret;
}

template<std::size_t Dim>
inline constexpr Vec<Dim> operator*(const Vec<Dim> &lhs, const Vec<Dim> &rhs) 
{
    auto ret{lhs};
    std::transform(lhs.cbegin(),lhs.cend(),
                   rhs.cbegin(), ret.begin(),
                   [](auto l, auto r) {return l*r;}
                  );          
    return ret;
}
template<std::size_t Dim>
inline constexpr Vec<Dim> operator*(float f, const Vec<Dim>& vec) 
{
    auto ret{vec};
    std::transform(vec.cbegin(),vec.cend(),
                   ret.begin(),
                   [f](auto v) {return f*v;}
                  );               
    return ret;
}

template<std::size_t Dim>
inline constexpr Vec<Dim> operator*(const Vec<Dim>& vec, float f) 
{
    return f*vec;
}

template<std::size_t Dim>
inline constexpr Vec<Dim> operator/(const Vec<Dim>& vec, float t)
{
    assert(t!=0);
    return (1.f/t)*vec;
}

template<std::size_t Dim>
inline constexpr Vec<Dim> UnitVector(const Vec<Dim>& vec)
{
    assert(vec.Length()!=0); 
    return vec/vec.Length();
}

template<std::size_t Dim>
inline constexpr float Dot(const Vec<Dim>& u, const Vec<Dim>& v)
{
    return  std::inner_product(u.cbegin(),u.cend(),v.cbegin(),0);
}

inline constexpr Vec<3> Cross(const Vec<3>& u, const Vec<3>& v)
{
    return Vec<3>(u[1]*v[2]-u[2]*v[1],
                u[2]*v[0]-u[0]*v[2], 
                u[0]*v[1]-u[1]*v[0]);
}

/// @brief A norm3 is just a vector that is guaranteed to be of unit length.
/// @brief Using the type system like this allows us to make guarantees about the data. 
class Norm3 : public Vec<3> {
public:

     /// @brief Constructs a normalised vec3 from a regular vec3
     /// @param vec Vec3 that should be normalised
     explicit constexpr Norm3(const Vec<3>& vec )
        : Vec<3>::Vec(UnitVector(vec)) {}//{assert(this->LengthSquared() == 1);} //TO-DO Do we need this assertion?
};

#endif