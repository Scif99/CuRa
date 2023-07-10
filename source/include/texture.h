#ifndef TEXTURE_H
#define TEXTURE_H

#include <string_view>
#include "buffer.h"

//TODO Better error handling (exceptions?)
Buffer<Color3f> ParsePPMTexture(std::string_view filename) {
    //#1 
    if(!filename.ends_with(".ppm")) {
        std::cerr<<"incorrect file format\n";
    }

    std::ifstream file(filename.data());
    if(!file) {
        std::cerr<<"Error loading file\n";
    }

    //the first line is the format
    std::string s;
    file>>s;
    if(s!="P3") {
        std::cerr<<"Wrong file type\n";
    }

    //The next two words are the width and height of the image respectively
    file>>s;
    auto width = std::stoi(s);
    file>>s;
    auto height = std::stoi(s);

    //The next word is the colour range  (should be 255)
    file>>s;
    if(s!="255") {
        std::cerr<<"Wrong color range\n";
    }

    //Now that we know the dimensions, we can create a buffer to extract the data into
    Buffer<Color3f> buffer(height,width);

    //Now we read in three values at a time, r,g,b.
    //Scale the values to the range [0,1]
    std::string r,g,b;
    int idx = 0;
    while(file>>r>>g>>b) {
        Color3f col{std::stof(r)/255.f,std::stof(g)/255.f,std::stof(b)/255.f};
        buffer[idx++] = col;
    }
    return buffer;
};

#endif