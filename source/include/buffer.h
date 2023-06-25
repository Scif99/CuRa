#ifndef BUFFER_H
#define BUFFER_H

#include <cassert>
#include <concepts>
#include <vector>
#include "vec.h"

/// @brief A buffer represents a contiguous block of data
/// @brief Follows the 'top-left origin' convention
/// @tparam T Type of the data stored

template<typename DataType>
requires (std::same_as<DataType,float> || std::same_as<DataType,Vec3f> || std::same_as<DataType,Vec3d>)
class Buffer {
private:
    std::vector<DataType> data_;
    int height_;
    int width_;

public:
    //Construct with empty values
    Buffer(int height, int width)
        : height_{height}, width_{width} {data_.resize(height*width);}

    //Construct with default values
    Buffer(int height, int width, const DataType& def)
        : data_(height*width, def), height_{height}, width_{width} {}

    int Height() const noexcept{return height_;}
    int Width() const noexcept {return width_;}
    
    /// @brief Set a specific pixel to an RGB color. Note that origin is at the top left rather than bottom left
    /// @param row Row in image
    /// @param x x coordinate in pixel space
    /// @param y y coordinate in pixel space 
    void Set(int x, int y, const DataType& val) {
        data_[y*width_+ x] = val;
    }
    
    DataType Get(int x, int y) const {
        return data_[y*width_+ x];
    }

    DataType& operator[](int i) {return  data_[i];};
    const DataType& operator[](int i) const {return data_[i];};

    //Iterators
    auto begin() { return data_.begin(); }
    auto end() { return data_.end(); }
    auto begin() const { return data_.begin(); }
    auto end() const { return data_.end(); }
    auto cbegin() const { return data_.cbegin(); }
    auto cend() const { return data_.cend(); }
};

#endif