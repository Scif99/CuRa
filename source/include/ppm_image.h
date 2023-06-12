#ifndef PPM_IMAGE_H
#define PPM_IMAGE_H

#include <array>
#include <iostream>
#include <fstream>
#include <variant>
#include <vector>

class PPMImage {
private:

    int height_;
    int width_;
public:    
    std::vector<Color> data_; //image data. Note that the color range is [0,1] and not [0,255]

public:
    PPMImage(int height, int width)
    :  height_{height}, width_{width} {data_.resize((height+1)*(width+1));}


    void Write(std::ofstream& out){
    out<<"P3\n"<<width_<<" "<<height_<<"\n255\n"; 
    for(const auto& pixel : data_) {
        const auto&[r,g,b] = pixel.Data(); //Get current pixel
        out<< static_cast<int>(255.999*r)<< " "<< static_cast<int>(255.999*g)<<" "<<static_cast<int>(255.999*b)<<'\n'; //Scale and write to file
    }
}

    /// @brief Set a specific pixel to an RGB color
    /// @param row Row in image
    /// @param x x coordinate in pixel space
    /// @param y y coordinate in pixel space
    void Set(int x, int y, const Color& val) {
        assert(y*width_ + x < data_.size());
        data_[y*width_+ x] = val;
    }


    Color Get(int row, int col) const {
        return data_[row*width_+ col];
    }

    void Fill(const Color& col) {
        for(auto& pixel : data_) pixel = col;
    }

    int Height() const {return height_;};
    int Width() const {return width_;};
    
};

#endif