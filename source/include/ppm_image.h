#ifndef PPM_IMAGE_H
#define PPM_IMAGE_H

#include <array>
#include <iostream>
#include <fstream>
#include <variant>
#include <vector>

// struct ColorVisitor {


// };

// struct Color {
//     std::variant<std::array<float,3>,
//                  bool> data_;
// };



class PPMImage {
private:
    enum class Format {
        BW,
        RGB
    };

    int height_;
    int width_;
    std::vector<std::array<float,3>> data_; //image data. Note that the color range is [0,1] and not [0,255]

public:
    PPMImage(int height, int width, Format = Format::RGB)
    :  height_{height}, width_{width} {data_.resize(height*width);}



    /// @brief Writes image data to a file
    /// @param out File stream
    void Write(std::ofstream& out);

    /// @brief Set a specific pixel to an RGB color
    /// @param row Row in image
    /// @param col 
    /// @param val 
    void Set(int row, int col, std::array<float, 3> val) {
        data_[row*width_+ col] = val;
    }

    /// @brief Returns the color of a specified pixel
    /// @param row Row in image
    /// @param col 
    /// @param val 
    std::array<float,3> Get(int row, int col, std::array<float, 3> val) {
        data_[row*width_+ col] = val;
    }

    void Fill(const std::array<float,3>& col) {
        for(auto& pixel : data_) pixel = col;
    }


};


inline void PPMImage::Write(std::ofstream& out) {

    out<<"P3\n"<<width_<<" "<<height_<<"\n255\n"; //TODO change for Black and white

    for(const auto& pixel : data_) {
        const auto&[r,g,b] = pixel; //Get current pixel
        out<< static_cast<int>(255.999*r)<< " "<< static_cast<int>(255.999*g)<<" "<<static_cast<int>(255.999*b)<<'\n'; //Scale and write to file
    }
    std::cerr<<"\nDone\n";
}




#endif