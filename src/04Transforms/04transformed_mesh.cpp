#include <iostream>
#include <vector>

#include <cura/buffer.h>
#include <cura/math.h>
#include <cura/model.h>
#include <cura/rasterizer.h>
#include <cura/transforms.h>
#include <cura/texture.h>
#include <cura/vertex.h>

static Color3f TextureLookup( const FrameBuffer& texture, float u, float v, bool flip_v = true) {
    assert(u>=0 && u<=1.f);
    assert(v>=0 && v<=1.f);

    //Also need to scale texture coordinates to the texture's dimensions
    const auto tw{texture.width};
    const auto th{texture.height}; 

    const float scaled_u = u*tw;
    const float scaled_v = flip_v ? th - v*th : v*th;
    return texture.Color(scaled_u,scaled_v);
}

//Similar to the previous iteration, except we now use the barycentric coordinates computed by the edge function to interpolate attributes over vertices
//In this case the attributes are depth and texture coordinates.
void DrawTriangle(const ClippedVertex& cv0,const ClippedVertex& cv1,const ClippedVertex& cv2, FrameBuffer& image, const FrameBuffer& texture) {

    const auto v0 = cv0.pixel_coords;
    const auto v1 = cv1.pixel_coords;
    const auto v2 = cv2.pixel_coords;

    //Determine bounding box of triangle
    std::int32_t minX = std::min({v0.x,v1.x, v2.x}); 
    std::int32_t maxX = std::max({v0.x,v1.x, v2.x}); 

    std::int32_t minY = std::min({v0.y,v1.y, v2.y}); 
    std::int32_t maxY = std::max({v0.y,v1.y, v2.y}); 

    // Dont need to draw anything outside screen bounds
    minX = std::max(minX, 0);
    minY = std::max(minY,0);

    maxX = std::min(maxX, image.width);
    maxY = std::min(maxY, image.height);

    static int r = 0;

    for(auto y = minY; y <= maxY; ++y )
    {
        for(auto x = minX; x <= maxX; ++x) 
        {
            const auto p = Vec2f(x,y); //Current pixel being tested

            if (auto bary_coords = oBarycentrics(cv0.pixel_coords.xy(),cv1.pixel_coords.xy(),cv2.pixel_coords.xy(),p); bary_coords) {
                const auto& [l0,l1,l2] = bary_coords.value(); //unpack barycentric coordinates
                
                //Interpolate depth (first so we can discard early if necessary)
                const float d  = l0*cv0.pixel_coords.z + l1*cv1.pixel_coords.z + l2*cv2.pixel_coords.z;
                if(d<image.Depth(x,y)) continue;
                image.Depth(x,y) = d;
                
                //Interpolate textures
                Vec2f tex_coords = l0*cv0.tex_coords + l1*cv1.tex_coords + l2*cv2.tex_coords;
                image.Color(x,y) =  TextureLookup(texture,tex_coords.x,tex_coords.y);
            }
        }
    }
}

//Draw a mesh using a texture for coloring.
int main() {

	constexpr int kheight{800};
	constexpr int kwidth{800};

	FrameBuffer image{kheight,kwidth};
	std::ofstream out_file{"/home/sc2046/Projects/Graphics/CuRa/scenes/03BarycentricInterpolation/textured_mesh.ppm"};


    const Model head("/home/sc2046/Projects/Graphics/CuRa/assets/models/head.obj");
    const FrameBuffer diffuse_map =  ParsePPMTexture("/home/sc2046/Projects/Graphics/CuRa/assets/textures/head_diffuse.ppm");


    //Iterate over each face (triangle) in the model
    for(const auto& face : head.Faces()) {

        std::array<Vertex,3> vertices;

        for(int i =0;i<3;++i) {

            const auto clippos = head.Vertices()[face.pos_idx[i]]; //position of the vertex in clip space
            Vec3f viewpos{(clippos.x+1)*kwidth/2.f, (-clippos.y+1)*kheight/2.f, clippos.z}; //position of vertex in viewport space (i.e pixel coords)

            const auto texcoord = head.TexCoords()[face.tex_idx[i]];

            cvertices[i] = ClippedVertex{viewpos, texcoord};
        }
        DrawTriangle(cvertices[0],cvertices[1],cvertices[2], image, diffuse_map);
    }

	if(!out_file) {std::cerr<<"Error creating file\n"; return 1;};
	image.WriteColorsPPM(out_file);
}


//Get a list of 