#ifndef MODEL_H
#define MODEL_H

#include <array>
#include <sstream>
#include <string>
#include <string_view>
#include <fstream>
#include <vector>
#include "vec.h"

/// @brief Splits a line into words by a specified delimiter
/// @param line String to be split
/// @param delimiter The words will be split using this. By default splits using whitespace
/// @return A vector containing the words
static std::vector<std::string> Split(std::string line, char delimiter=' ') {
    std::vector<std::string> words;
    std::istringstream iss(line.data());
    for(std::string token; std::getline(iss,token, ' ');) {
        words.push_back(token);
    }
    return words;
}

class Model {
private:
    std::vector<Vec3> vertices_;
    std::vector<std::vector<int>> faces_;

    void Parse(std::string_view filename);
    Vec3 ParseOBJVertexPos(std::string_view line); //Parses a geometric vertex line from an obj file
    std::vector<int> ParseOBJFaceIndices(std::string_view line);

public:


    Model(std::string_view path) {
        Parse(path);
    }

    const std::vector<Vec3>& Vertices() const noexcept{return vertices_;};
    const std::vector<std::vector<int>>& Faces() const noexcept{return faces_;}; //TODO switch to array?


};

//Example of input 'v 0.123 0.234 0.345 1.0'
//We only care about the first 3 numbers (x,y,z)
Vec3 Model::ParseOBJVertexPos(std::string_view line) {
    auto words = Split(line.data(), ' ');

    return Vec3{std::stof(words[1]),std::stof(words[2]),std::stof(words[3])}; //only want the 3 vertices
}

//Example of input 'f 6/4/1 3/5/3 7/6/5'
//The important info is the first number in each word, representing the index of the vertex
std::vector<int> Model::ParseOBJFaceIndices(std::string_view line) {

    auto words = Split(line.data(), ' ');

    //Each element is now something like '6/4/1'.
    //We now extract the first number (the vertex index)
    std::vector<int> face;
    for(auto it = words.begin()+1; it!=words.end();++it) {
        auto v_index = Split(*it,'/')[0]; //The first token is the vertex index
        face.push_back(std::stoi(v_index) - 1);
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
        if(curr_line.compare(0,2,"f ")==0) {
            faces_.push_back(ParseOBJFaceIndices(curr_line));
        }
    }
};

#endif