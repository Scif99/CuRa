#ifndef PPM_IMAGE_H
#define PPM_IMAGE_H

#include <iostream>
#include <fstream>


#include "buffer.h"
#include "vec.h"

#include <string_view>
#include <string>


class PPMImage {
private:    
    Buffer<Color3f> buffer_; //image data. Note that the color range is [0,1] and not [0,255]
public:
    PPMImage(int height, int width)
    :  buffer_{height, width} {}

    PPMImage(const Buffer<Color3f>& buffer)
    :  buffer_{buffer} {}

    PPMImage(Buffer<Color3f>&& buffer)
    :  buffer_{std::move(buffer)} {}

    int Height() const noexcept{return buffer_.Height();}
    int Width() const noexcept {return buffer_.Width();}

    Color3f Get(int x, int y) const {
        return buffer_.Get(x, y);
    }

    void Write(std::ofstream& out){
    out<<"P3\n"<<Height()<<" "<<Width()<<"\n255\n"; 
    for(const auto& pixel : buffer_) {
        const auto&[r,g,b] = pixel.Data(); //Get current pixel
        out<< static_cast<int>(255.999*r)<< " "<< static_cast<int>(255.999*g)<<" "<<static_cast<int>(255.999*b)<<'\n'; //Scale and write to file
    }
}

};

#endif