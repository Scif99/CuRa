
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
        for(int i =0;i<4;++i) {this->operator()(i,i)=1.f;}
    }

    float& operator()(int i, int j) {return data_[i*4 + j];}
    const float& operator()(int i, int j) const {return data_[i*4 + j];}

    
    //Iterators
    auto begin() { return data_.begin(); }
    auto end() { return data_.end(); }
};


Mat4f operator*(const Mat4f& lhs, const Mat4f& rhs) {
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

[[nodiscard]] Vec4f operator*(const Mat4f& lhs, const Vec4f& rhs) {
    Vec4f ans;
    for(int i =0; i<4;++i) {
        for(int j = 0;j<4;++j) {
            ans[i]+= lhs(i,j)*rhs[j];
        }
    }
    return ans;
};