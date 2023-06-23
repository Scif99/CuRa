#ifndef MAT_H
#define MAT_H
#include <array>
#include <iostream>
#include "vec.h"
#include <optional>


class Mat4f{
private:
    std::array<float, 16> data_;

    static constexpr int rows{4};
    static constexpr int cols{4};

public:

    Mat4f() {
        std::fill(data_.begin(),data_.end(),0.f);
        //for(int i =0;i<4;++i) {this->operator()(i,i)=1.f;}
    }

    explicit Mat4f(const std::array<float,16>& elem)
        :data_{elem} {}

    explicit Mat4f(std::array<float,16>&& elem)
        :data_{std::move(elem)} {}

    float& operator()(int i, int j) {return data_[i*4 + j];}
    const float& operator()(int i, int j) const {return data_[i*4 + j];}

    static Mat4f Identity() {
        Mat4f Id;
        for(int i =0;i<4;++i) {Id(i,i)=1.f;}
        return Id;
    }
    

    //Iterators
    auto begin() { return data_.begin(); }
    auto end() { return data_.end(); }
};



[[nodiscard]] inline Mat4f operator*(const Mat4f& lhs, const Mat4f& rhs) {
    Mat4f ans;
    for(int i =0;i<4;++i) {
        for(int j = 0; j<4;++j) {
            ans(i,j) = 0.f;
            for(int k = 0;k<4;++k) {
                ans(i,j)+=lhs(i,k)*rhs(k,j);
            }
        }
    }
    return ans;
};

[[nodiscard]] inline Vec4f operator*(const Mat4f& lhs, const Vec4f& rhs) {
    Vec4f ans;
    for(int i =0; i<4;++i) {
        for(int j = 0;j<4;++j) {
            ans[i]+= lhs(i,j)*rhs[j];
        }
    }
    return ans;
};


//Creates a 4x4 matrix a representing a translation by v 
[[nodiscard]] inline Mat4f Translate(const Mat4f& a, const Vec3f& v) {

    std::array<float,16> data = {
        1.f, 0.f, 0.f, v.X(),
        0.f, 1.f, 0.f, v.Y(),
        0.f, 0.f, 1.f, v.Z(),
        0.f, 0.f, 0.f, 1.f
    };
    Mat4f tr = Mat4f(std::move(data));
    return tr*a;
}

//Creates a 4x4 matrix a representing a translation by v 
[[nodiscard]] inline Mat4f Scale(const Mat4f& a, const Vec3f& v) {

    std::array<float,16> data = {
        v.X(), 0.f,   0.f,   0.f,
        0.f,   v.Y(), 0.f,   0.f,
        0.f,   0.f,   v.Z(), 0.f,
        0.f,   0.f,   0.f,   1.f
    };
    Mat4f tr = Mat4f(std::move(data));
    return tr*a;
}

[[nodiscard]] inline  Mat4f Transpose(const Mat4f& m) {
    Mat4f tr;
    for(int i=0;i<4;++i) {
        for(int j = 0;j<4; ++j) {
            tr(i,j) = m(j,i);
        }
    }
    return tr;
}

//Yoinked from https://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
[[nodiscard]] inline std::optional<Mat4f> Invert(const Mat4f& m)
 {
    Mat4f inv_m;
        inv_m(0,0) = m(1,1) * m(2,2) * m(3,3) -
                         m(1,1) * m(2,3) * m(3,2) -
                         m(2,1) * m(1,2) * m(3,3) +
                         m(2,1) * m(1,3) * m(3,2) +
                         m(3,1) * m(1,2) * m(2,3) -
                         m(3,1) * m(1,3) * m(2,2);

        inv_m(1,0) = -m(1,0) * m(2,2) * m(3,3) +
                         m(1,0) * m(2,3) * m(3,2) +
                         m(2,0) * m(1,2) * m(3,3) -
                         m(2,0) * m(1,3) * m(3,2) -
                         m(3,0) * m(1,2) * m(2,3) +
                         m(3,0) * m(1,3) * m(2,2);

        inv_m(2,0) = m(1,0) * m(2,1) * m(3,3) -
                         m(1,0) * m(2,3) * m(3,1) -
                         m(2,0) * m(1,1) * m(3,3) +
                         m(2,0) * m(1,3) * m(3,1) +
                         m(3,0) * m(1,1) * m(2,3) -
                         m(3,0) * m(1,3) * m(2,1);

        inv_m(3,0) = -m(1,0) * m(2,1) * m(3,2) +
                         m(1,0) * m(2,2) * m(3,1) +
                         m(2,0) * m(1,1) * m(3,2) -
                         m(2,0) * m(1,2) * m(3,1) -
                         m(3,0) * m(1,1) * m(2,2) +
                         m(3,0) * m(1,2) * m(2,1);

        inv_m(0,1) = -m(0,1) * m(2,2) * m(3,3) +
                         m(0,1) * m(2,3) * m(3,2) +
                         m(2,1) * m(0,2) * m(3,3) -
                         m(2,1) * m(0,3) * m(3,2) -
                         m(3,1) * m(0,2) * m(2,3) +
                         m(3,1) * m(0,3) * m(2,2);

        inv_m(1,1) = m(0,0) * m(2,2) * m(3,3) -
                         m(0,0) * m(2,3) * m(3,2) -
                         m(2,0) * m(0,2) * m(3,3) +
                         m(2,0) * m(0,3) * m(3,2) +
                         m(3,0) * m(0,2) * m(2,3) -
                         m(3,0) * m(0,3) * m(2,2);

        inv_m(2,1) = -m(0,0) * m(2,1) * m(3,3) +
                         m(0,0) * m(2,3) * m(3,1) +
                         m(2,0) * m(0,1) * m(3,3) -
                         m(2,0) * m(0,3) * m(3,1) -
                         m(3,0) * m(0,1) * m(2,3) +
                         m(3,0) * m(0,3) * m(2,1);

        inv_m(3,1) = m(0,0) * m(2,1) * m(3,2) -
                         m(0,0) * m(2,2) * m(3,1) -
                         m(2,0) * m(0,1) * m(3,2) +
                         m(2,0) * m(0,2) * m(3,1) +
                         m(3,0) * m(0,1) * m(2,2) -
                         m(3,0) * m(0,2) * m(2,1);

        inv_m(0,2) = m(0,1) * m(1,2) * m(3,3) -
                         m(0,1) * m(1,3) * m(3,2) -
                         m(1,1) * m(0,2) * m(3,3) +
                         m(1,1) * m(0,3) * m(3,2) +
                         m(3,1) * m(0,2) * m(1,3) -
                         m(3,1) * m(0,3) * m(1,2);

        inv_m(1,2) = -m(0,0) * m(1,2) * m(3,3) +
                         m(0,0) * m(1,3) * m(3,2) +
                         m(1,0) * m(0,2) * m(3,3) -
                         m(1,0) * m(0,3) * m(3,2) -
                         m(3,0) * m(0,2) * m(1,3) +
                         m(3,0) * m(0,3) * m(1,2);

        inv_m(2,2) = m(0,0) * m(1,1) * m(3,3) -
                         m(0,0) * m(1,3) * m(3,1) -
                         m(1,0) * m(0,1) * m(3,3) +
                         m(1,0) * m(0,3) * m(3,1) +
                         m(3,0) * m(0,1) * m(1,3) -
                         m(3,0) * m(0,3) * m(1,1);

        inv_m(3,2) = -m(0,0) * m(1,1) * m(3,2) +
                         m(0,0) * m(1,2) * m(3,1) +
                         m(1,0) * m(0,1) * m(3,2) -
                         m(1,0) * m(0,2) * m(3,1) -
                         m(3,0) * m(0,1) * m(1,2) +
                         m(3,0) * m(0,2) * m(1,1);

        inv_m(0,3) = -m(0,1) * m(1,2) * m(2,3) +
                         m(0,1) * m(1,3) * m(2,2) +
                         m(1,1) * m(0,2) * m(2,3) -
                         m(1,1) * m(0,3) * m(2,2) -
                         m(2,1) * m(0,2) * m(1,3) +
                         m(2,1) * m(0,3) * m(1,2);

        inv_m(1,3) = m(0,0) * m(1,2) * m(2,3) -
                         m(0,0) * m(1,3) * m(2,2) -
                         m(1,0) * m(0,2) * m(2,3) +
                         m(1,0) * m(0,3) * m(2,2) +
                         m(2,0) * m(0,2) * m(1,3) -
                         m(2,0) * m(0,3) * m(1,2);

        inv_m(2,3) = -m(0,0) * m(1,1) * m(2,3) +
                         m(0,0) * m(1,3) * m(2,1) +
                         m(1,0) * m(0,1) * m(2,3) -
                         m(1,0) * m(0,3) * m(2,1) -
                         m(2,0) * m(0,1) * m(1,3) +
                         m(2,0) * m(0,3) * m(1,1);

        inv_m(3,3) = m(0,0) * m(1,1) * m(2,2) -
                         m(0,0) * m(1,2) * m(2,1) -
                         m(1,0) * m(0,1) * m(2,2) +
                         m(1,0) * m(0,2) * m(2,1) +
                         m(2,0) * m(0,1) * m(1,2) -
                         m(2,0) * m(0,2) * m(1,1);

        double det = m(0,0) * inv_m(0,0) +
                     m(0,1) * inv_m(1,0) +
                     m(0,2) * inv_m(2,0) +
                     m(0,3) * inv_m(3,0);

        if (det == 0)
            return std::nullopt;

        det = 1.0 / det;

        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                inv_m(i,j) = inv_m(i,j) * det;
            }
        }

        return inv_m;
    }

#endif