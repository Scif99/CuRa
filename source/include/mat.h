
#include <array>

#include "vec.h"


class Mat4f{
private:
    std::array<float, 16> data_;

    static constexpr int Rows{4};
    static constexpr int Cols{4};

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
    
    void Transpose();

    //Iterators
    auto begin() { return data_.begin(); }
    auto end() { return data_.end(); }
};

inline void Mat4f::Transpose() {
    for(int i=0;i<4;++i) {
        for(int j = 0;j<4; ++j) {
            this->operator()(i,j) = this->operator()(j,i);
        }
    }
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

[[nodiscard]] inline  Vec4f operator*(const Mat4f& lhs, const Vec4f& rhs) {
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