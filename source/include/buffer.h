#ifndef BUFFER_H
#define BUFFER_H

#include <cassert>
#include <concepts>
#include <fstream>
#include <mutex>
#include <vector>

#include "vec.h"

/// @brief A buffer models a 2D block of data.
/// @brief Follows the 'top-left origin' convention
/// @tparam T Type of the data stored
template<typename DataType>
requires (std::same_as<DataType,float> || std::same_as<DataType,Vec3f> || std::same_as<DataType,Norm3f>)
class Buffer {
private:
    std::vector<DataType> data_;
    int height_;
    int width_;
    //std::mutex mtx;

public:
    //Construct with empty values
    Buffer(int height, int width)
        : height_{height}, width_{width} {data_.resize(height*width);}

    //Construct with default values
    Buffer(int height, int width, const DataType& def)
        : data_(height*width, def), height_{height}, width_{width} {}

    int Height() const noexcept{return height_;}
    int Width() const noexcept {return width_;}

    void Set(int x, int y, const DataType& val) {
        //const std::lock_guard<std::mutex> lock(mtx);
        data_[y*width_+ x] = val;
    }
        
    [[nodiscard]] DataType Get(int x, int y) const {
        //const std::lock_guard<std::mutex> lock(mtx);
        return data_[y*width_+ x];
    }

    //Direct accessors. Not thread-safe.
    DataType& operator[](int i) {return  data_[i];};
    const DataType& operator[](int i) const {return data_[i];};

    void Write(std::ofstream& out) requires(std::same_as<DataType,Vec3f>){
        out<<"P3\n"<<Height()<<" "<<Width()<<"\n255\n"; 
        for(const auto& pixel : data_) {
        const auto&[r,g,b] = pixel.Data(); //Get current pixel
        out<< static_cast<int>(255.999*r)<< " "<< static_cast<int>(255.999*g)<<" "<<static_cast<int>(255.999*b)<<'\n'; //Scale and write to file
        }
    }

    void Write(std::ofstream& out) requires(std::same_as<DataType,float>){
        out<<"P3\n"<<Height()<<" "<<Width()<<"\n255\n"; 
        for(const auto& pixel : data_) {
            out<<pixel<<'\n';
        }
    }

    //Iterators
    auto begin() { return data_.begin(); }
    auto end() { return data_.end(); }
    auto begin() const { return data_.begin(); }
    auto end() const { return data_.end(); }
    auto cbegin() const { return data_.cbegin(); }
    auto cend() const { return data_.cend(); }
};

#endif