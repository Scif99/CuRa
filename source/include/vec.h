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


template<typename T>
concept Decimal = std::is_floating_point_v<T>;


//A generic 2D/3D vector class for different floating point precisions.
template<Decimal Prec, std::size_t Dim>
requires(Dim==2 || Dim==3 || Dim==4)
class Vec {
protected:
    std::array<Prec,Dim> elem;
public:
    //Constructor for 2D vectors
    explicit constexpr Vec() requires(Dim==2) = default;
    explicit constexpr Vec(Prec x, Prec y) requires(Dim==2) 
        : elem{x,y} {}
    
    //Constructor for 3D vectors
    explicit constexpr Vec() requires(Dim==3) = default;
    explicit constexpr Vec(Prec x, Prec y, Prec z) requires(Dim==3) 
        : elem{x,y,z} {}

    //Constructor for 4D vectors
    explicit constexpr Vec() requires(Dim==4) = default;
    explicit constexpr Vec(Prec x, Prec y, Prec z, Prec w) requires(Dim==4) 
        : elem{x,y,z,w} {}

    explicit constexpr Vec(Vec<Prec, 3> v3, Prec w) requires(Dim==4)
        : elem{v3[0],v3[1],v3[2],w} {} 
    

    constexpr Prec X() const noexcept {return elem[0];}
    constexpr Prec Y() const noexcept {return elem[1];}
    constexpr Prec Z() const noexcept requires(Dim>=3)  {return elem[2];}
    constexpr Prec W() const noexcept requires(Dim==4)  {return elem[3];}


    constexpr Prec R() const noexcept requires(Dim==3) {return elem[0];}
    constexpr Prec G() const noexcept requires(Dim==3) {return elem[1];}
    constexpr Prec B() const noexcept requires(Dim==3) {return elem[2];}

    constexpr Prec U() const noexcept requires(Dim==2) {return elem[0];}
    constexpr Prec V() const noexcept requires(Dim==2) {return elem[1];}

    constexpr Vec operator-() const noexcept {
        auto ret(*this);
        std::transform(this->cbegin(),this->cend(),
                   ret.begin(),
                   [](auto v) {return -v;});
        return ret;
    }

    constexpr Prec& operator[](int i) {return elem[i];}    
    constexpr const Prec& operator[](int i) const {return elem[i];}

    
    constexpr Vec& operator+=(const Vec& other) {
        std::transform(this->cbegin(),this->cend(), 
                       other.cbegin(), this->begin(),
                       [](auto l, auto r){return l+r;});
        return *this;
    }
    constexpr Vec& operator*=(const Prec t)  {
        std::transform(this->cbegin(),this->cend(), 
                        this->begin(),
                       [t](auto e){return e*t;});
        return *this;
    }
    
    constexpr Vec& operator/=(const Prec t)  {
        assert(t!=0);
        return *this *= 1/t;
    }

    constexpr Prec LengthSquared() const {return std::inner_product(this->cbegin(),this->cend(),this->cbegin(), static_cast<Prec>(0));}
    constexpr Prec Length() const {return static_cast<Prec>(sqrtl(LengthSquared()));}


    const auto& Data() const {return elem;}

    //Iterators
    auto begin() { return elem.begin(); }
    auto end() { return elem.end(); }
    auto cbegin() const { return elem.begin(); }
    auto cend() const { return elem.end(); }
    auto begin() const { return elem.begin(); }
    auto end() const { return elem.end();}

    template <Decimal T, std::size_t S>      // all instantiations of this template are my friends
    friend constexpr bool operator==(const Vec<T,S>& lhs, const Vec<T,S>& rhs);

};


//Aliases
using Vec2f = Vec<float, 2>;
using Point2f = Vec<float, 2>;
using Vec3f = Vec<float,3>;
using Color3f = Vec<float, 3>;
using Point3f = Vec<float, 3>;
using Vec4f = Vec<float,4>;


using Vec2d = Vec<double, 2>;
using Point2d = Vec<double, 2>;
using Vec3d = Vec<double,3>;
using Color3d = Vec<double, 3>;
using Point3d = Vec<double, 3>;

    constexpr Vec3f Cartesian(const Vec4f& h) {return Vec3f{h[0],h[1],h[2]};}

template<Decimal Prec, std::size_t Dim>
inline constexpr bool operator == (const Vec<Prec, Dim>& lhs, const Vec<Prec, Dim>& rhs)
{
    return lhs.elem == rhs.elem;
}

template<Decimal Prec, std::size_t Dim>
inline constexpr auto operator+(const Vec<Prec, Dim>& lhs, const Vec<Prec, Dim>& rhs) 
{
    auto ret{lhs};
    std::transform(lhs.cbegin(),lhs.cend(),
                   rhs.cbegin(), ret.begin(),
                   [](auto l, auto r) {return l+r;}
                  );
    return ret;
}

template<Decimal Prec, std::size_t Dim>
inline constexpr auto operator-(const Vec<Prec, Dim>& lhs, const Vec<Prec, Dim>& rhs) 
{
    auto ret{lhs};
    std::transform(lhs.cbegin(),lhs.cend(),
                   rhs.cbegin(), ret.begin(),
                   []( auto l, auto r) {return l-r;}
                  );          
    return ret;
}

//Element-wise multiplication of two vectors of the same dimension
template<Decimal Prec, std::size_t Dim>
inline constexpr auto operator*(const Vec<Prec, Dim> &lhs, const Vec<Prec, Dim> &rhs) 
{
    auto ret{lhs};
    std::transform(lhs.cbegin(),lhs.cend(),
                   rhs.cbegin(), ret.begin(),
                   [](auto l, auto r) {return l*r;}
                  );          
    return ret;
}

template<Decimal Prec, std::size_t Dim>
inline constexpr auto operator*(Prec f, const Vec<Prec, Dim>& vec) 
{
    auto ret{vec};
    std::transform(vec.cbegin(),vec.cend(),
                   ret.begin(),
                   [f](auto v) {return f*v;}
                  );               
    return ret;
}

template<Decimal Prec, std::size_t Dim>
inline constexpr auto operator*(const Vec<Prec, Dim>& vec, Prec f) 
{
    return f*vec;
}

template<Decimal Prec, std::size_t Dim>
inline constexpr auto operator/(const Vec<Prec, Dim>& vec, Prec t)
{
    assert(t!=0);
    return (1.f/t)*vec;
}

template<Decimal Prec, std::size_t Dim>
inline constexpr auto UnitVector(const Vec<Prec, Dim>& vec)
{
    assert(vec.Length()!=0.f); 
    return vec/vec.Length();
}

template<Decimal Prec, std::size_t Dim>
inline constexpr auto Dot(const Vec<Prec, Dim>& u, const Vec<Prec, Dim>& v)
{
    return  std::inner_product(u.cbegin(),u.cend(),v.cbegin(),static_cast<Prec>(0.f));
}

template<Decimal Prec, std::size_t Dim>
inline constexpr auto Cross(const Vec<Prec, 3>& u, const Vec<Prec, 3>& v)
{
    return Vec<Prec,3>(u[1]*v[2]-u[2]*v[1],
                  u[2]*v[0]-u[0]*v[2], 
                  u[0]*v[1]-u[1]*v[0]);
}

/// @brief A norm3 is just a vector that is guaranteed to be of unit length.
/// @brief Using the type system like this allows us to make guarantees about the data. 
class Norm3f : public Vec<float, 3> {
public:

     /// @brief Constructs a normalised vec3 from a regular vec3
     /// @param vec Vec3 that should be normalised
     explicit constexpr Norm3f(const Vec<float,3>& vec )
        : Vec<float, 3>::Vec(UnitVector(vec)) {}//{assert(this->LengthSquared() == 1);} //TO-DO Do we need this assertion?
};


#endif