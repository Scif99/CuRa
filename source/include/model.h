#ifndef MODEL_H
#define MODEL_H

#include <array>
#include <sstream>
#include <string>
#include <string_view>
#include <fstream>
#include <ranges>
#include <vector>
#include "vec.h"

/// @brief Splits a line into words by a specified delimiter
/// @param line String to be split
/// @param delimiter The words will be split using this. By default splits using whitespace
/// @return A vector containing the words
static std::vector<std::string> Split(std::string line, char delimiter=' ') {
    std::vector<std::string> words;
    std::istringstream iss(line.data());
    for(std::string token; std::getline(iss,token, delimiter);) {
        if(!token.empty()) {
        words.push_back(token);
        }
    }
    return words;
}

//A face contains the indices to vertex attributes such as position, texture coordinates etc
// e.g. pos_idx[0] is the index to the vertex position of the first vertex
struct Face {
    std::vector<int> pos_idx;
    std::vector<int> tex_idx;
    std::vector<int> norm_idx;

};

class Model {
private:
    std::vector<Vec3f> vertices_;
    std::vector<Vec2f> tex_coords_;
    std::vector<Vec3f> normals_;
    std::vector<Face> faces_;

    void Parse(std::string_view filename);
    Vec3f ParseOBJVertexPos(std::string_view line); //Parses a geometric vertex line from an obj file
    Vec2f ParseOBJTexCoords(std::string_view line); 
    Vec3f ParseOBJVertexNorm(std::string_view line);
    Face ParseOBJFaceIndices(std::string_view line);
    


public:
    Model(std::string_view path) {
        Parse(path);
    }
    const std::vector<Vec3f>& Vertices() const noexcept{return vertices_;};
    const std::vector<Vec2f>& TexCoords() const noexcept{return tex_coords_;}   
    const std::vector<Vec3f>& Normals() const noexcept{return normals_;};

    const std::vector<Face>& Faces() const noexcept{return faces_;}; //TODO switch to array?


};

//Example of input 'v 0.123 0.234 0.345 1.0'
//We only care about the first 3 numbers (x,y,z)
Vec3f Model::ParseOBJVertexPos(std::string_view line) {
    const auto words = Split(line.data(), ' ');

    return Vec3f{std::stof(words[1]),std::stof(words[2]),std::stof(words[3])}; //only want the 3 vertices
}


//Example: vt  0.532 0.923 0.000
//We only care about the first 2 numbers (u,v)
Vec2f Model::ParseOBJTexCoords(std::string_view line) {
    const auto words = Split(line.data(), ' ');
    return Vec2f{std::stof(words[1]),std::stof(words[2])}; 
}

//Example: vn  0.001 0.482 -0.876
Vec3f Model::ParseOBJVertexNorm(std::string_view line) {
    const auto words = Split(line.data(), ' ');
    return Vec3f{std::stof(words[1]),std::stof(words[2]), std::stof(words[3])}; 
}


//Example of input 'f 6/4/1 3/5/3 7/6/5'
//Example of output {6,3,7},{3,5,3}  ({position}, {tex_coord})
//Each 'word' stores indices to attribute data
//NOTE INDICES IN THE OBJ FILE START FROM 1
//Extracts the indices of the vertex position and texture coords
Face Model::ParseOBJFaceIndices(std::string_view line) {

    const auto words = Split(line.data(), ' ');

    Face face;
    for(auto it = words.begin()+1; it!=words.end();++it) {
        //Each element is now something like '6/4/1'.
        //We split again by '/' and then extract the data
        const auto attribs = Split(*it,'/');
        //we have now split '6/4/1' into  the sequence 6,4,2
        //Remember to subtract 1 from the index to get 0-indexing
        face.pos_idx.push_back(std::stoi(attribs[0]) - 1); //Position
        face.tex_idx.push_back(std::stoi(attribs[1]) - 1); //Texture coord
        face.norm_idx.push_back(std::stoi(attribs[2]) - 1); //Normal

    }

    return face; 
}


//Parses an obj file
//Fills the vertices and faces attributes
void Model::Parse(std::string_view filename) {
    if(!filename.ends_with(".obj")) {
        std::cerr<<"Incorrect file format.\n";
    }
    std::ifstream file(filename.data());
    if(!file) {
        std::cerr<<"Error loading file\n";
    }

    //Parse each line individually
    for(std::string curr_line; std::getline(file, curr_line);) {
        if(curr_line.compare(0,2,"v ")==0){
            vertices_.push_back(ParseOBJVertexPos(curr_line));
        }
        if(curr_line.compare(0,3,"vt ")==0){
            tex_coords_.push_back(ParseOBJTexCoords(curr_line));
        }

        if(curr_line.compare(0,3,"vn ")==0){
            normals_.push_back(ParseOBJVertexNorm(curr_line));
        }
        if(curr_line.compare(0,2,"f ")==0) {
            faces_.push_back(ParseOBJFaceIndices(curr_line));
        }
    }
};


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
    std::string r,g,b;
    int idx = 0;
    while(file>>r>>g>>b) {
        Color3f col{std::stof(r)/255.f,std::stof(g)/255.f,std::stof(b)/255.f};
        buffer[idx] = col;
        ++idx;
    }

    return buffer;
};

#endif