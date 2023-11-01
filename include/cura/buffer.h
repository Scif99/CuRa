#pragma once

#include <cassert>
#include <fstream>
#include <vector>
#include <cstdint>

#include <cura/math.h>

/// @brief A framebuffer is a 2D buffer that contains data used for rendering.
/// @brief Follows the 'top-left origin' convention
class FrameBuffer {
public:

    //Construct with empty values
    FrameBuffer(std::int32_t h, std::int32_t w)
        : height{h}, width{w} 
        {
            assert(h%2==0 && w%2==0 &&"Error: Framebuffer dimensions must be even!");

            colors.resize(h*w);
            depths.resize(h*w);
        }

    // helpers that look up colors and depths for sample s of pixel (x,y):
	Color3f& Color(std::int32_t x, std::int32_t y) {
		return colors[y*width+ x];
	}
	Color3f const& Color(std::int32_t x, std::int32_t y) const {
		return colors[y*width+ x];
	}
	float& Depth(std::int32_t x, std::int32_t y) {
		return depths[y*width+ x];
	}
	float const& Depth(std::int32_t x, std::int32_t y) const {
		return depths[y*width+ x];
	}

    //Write depth values to output stream in PPM format
    void WriteDepthsPPM(std::ofstream& out) {
        out<<"P3\n"<<height<<" "<<width<<"\n255\n"; 
        for(const auto& pixel : depths) {
            out<<pixel<<'\n';
        }
    }

    //Write color values to output stream in PPM format
    void WriteColorsPPM(std::ofstream& out) { 
        out<<"P3\n"<<height<<" "<<width<<"\n255\n"; 
        for(const auto& [r,g,b] : colors) {
            out<< static_cast<int>(255.999*r)<< " "<< static_cast<int>(255.999*g)<<" "<<static_cast<int>(255.999*b)<<'\n'; //Scale and write to file
            }
    }



public:
    std::int32_t height;
    std::int32_t width;
    std::vector<Color3f> colors;
    std::vector<float>   depths;
};

