#include <iostream>
#include <vector>

#include <cura/buffer.h>
#include <cura/camera.h>
#include <cura/math.h>
#include <cura/model.h>
#include <cura/rasterizer.h>
#include <cura/transforms.h>
#include <cura/texture.h>
#include <cura/vertex.h>
#include <cura/shader.h>


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

    maxX = std::min(maxX, image.width-1);
    maxY = std::min(maxY, image.height-1);

    for(auto y = minY; y <= maxY; ++y )
    {
        for(auto x = minX; x <= maxX; ++x) 
        {
            const auto p = Vec2f(x,y); //Current pixel being tested

            if (auto bary_coords = oBarycentrics(cv0.pixel_coords.xy(),cv1.pixel_coords.xy(),cv2.pixel_coords.xy(),p); bary_coords) {
                const auto& [b0,b1,b2] = bary_coords.value(); //unpack barycentric coordinates
                
                //Interpolate depth (first so we can discard early if necessary)
                const float d  = b0*cv0.pixel_coords.z + b1*cv1.pixel_coords.z + b2*cv2.pixel_coords.z;
                

                //Early depth testing
                if(d<image.Depth(x,y)) continue;
                image.Depth(x,y) = d;
                
                //Interpolate textures
                Vec2f tex_coords = b0*cv0.tex_coords + b1*cv1.tex_coords + b2*cv2.tex_coords;
                
                image.Color(x,y) =  TextureLookup(texture,tex_coords.x,tex_coords.y);
            }
        }
    }
}




//Draw a mesh using a texture for coloring.
int main() {

	constexpr int kheight{800};
	constexpr int kwidth{800};
    constexpr float kaspect_ratio{static_cast<float>(kwidth)/ static_cast<float>(kheight)};

    const Camera camera(
        {0.f,0.f,3.f}, //eye
        {0.f,0.f,0.f}, //centre
        {0.f,1.f,0.f}  //up
    );
    
	FrameBuffer image{kheight,kwidth};
	std::ofstream out_file{"/home/sc2046/Projects/Graphics/CuRa/scenes/05PerspectiveCorrectInterpolation/angled_triangle.ppm"};

    //Load model and the associated texture(s).
    //const Model head("/home/sc2046/Projects/Graphics/CuRa/assets/models/head.obj");
    //const FrameBuffer diffuse_map =  ParsePPMTexture("/home/sc2046/Projects/Graphics/CuRa/assets/textures/head_diffuse.ppm");

    const Model head("/home/sc2046/Projects/Graphics/CuRa/assets/models/angled_triangle.obj");
    const FrameBuffer diffuse_map = ParsePPMTexture("/home/sc2046/Projects/Graphics/CuRa/assets/textures/checker.ppm");

    //Iterate over each face (triangle) in the model
    for(const auto& face : head.Faces()) {

        std::array<ClippedVertex,3> clippedvertices;

        for(int i =0;i<3;++i) {

            //Get position of vertex in 3D world space and convert to homogeneous coordinates.
            const auto worldpos = head.Vertices()[face.pos_idx[i]]; 
            const auto hworldpos = Vec4f(worldpos,1.f);

            //Transform to camera space by applying view matrix.
            const auto lookat = camera.view;
            const auto hcamerapos = la::mul(lookat, hworldpos);

            //Transform to clip space by applying projection matrix.
            //const auto projection_matrix = OrthographicProjection(-1.f,1.f,-1.f,1.f,-1.f,-5.f);
            const auto projection_matrix = PerspectiveProjection(std::numbers::pi_v<float>/4.f, kaspect_ratio,-1.f,-5.f);
            
            const auto hclipspacepos = la::mul(projection_matrix,hcamerapos);

            //Clipping (ignored)

            //Transform to NDC by applying perspective divide. (Note this does nothing for an orthographic projection).
            const auto ndcpos = hclipspacepos.xyz() / hclipspacepos.w;

            //Transform to screen space by applying viewport transformation.
            //Keep the z coordinate for depth testing.
            const auto viewpos = Vec3f{(ndcpos.x+1)*kwidth/2.f, (-ndcpos.y+1)*kheight/2.f, ndcpos.z};
            //Also get other attributes from the model...
            const auto texcoord = head.TexCoords()[face.tex_idx[i]];

            clippedvertices[i] = ClippedVertex{viewpos, texcoord};
        }
        //Rasterise & color
        DrawTriangle(clippedvertices[0],clippedvertices[1],clippedvertices[2], image, diffuse_map);
    }

	if(!out_file) {std::cerr<<"Error creating file\n"; return 1;};
	image.WriteColorsPPM(out_file);
}