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
            


                //Evaluate 1/z at each vertex
                const auto inv_z0 = 1.f/cv0.clip_z;
                const auto inv_z1 = 1.f/cv1.clip_z;
                const auto inv_z2 = 1.f/cv2.clip_z;
                //Interpolate 1/z
                const auto inv_z_interp = b0*inv_z0 + b1*inv_z1 + b2*inv_z2;

                //Compute perspective-correct depth attribute
                const auto pCorrectDepth = 1.f/inv_z_interp;

                //Early depth testing
                if(pCorrectDepth<image.Depth(x,y)) continue;
                image.Depth(x,y) = pCorrectDepth;

                //Interpolate other attributes

                //Texture
                auto  pCorrectTex = b0*cv0.tex_coords*inv_z0  + b1*cv1.tex_coords*inv_z1  + b2*cv2.tex_coords*inv_z2;
                pCorrectTex /= (1.f/pCorrectDepth);

                image.Color(x,y) =  TextureLookup(texture,pCorrectTex.x,pCorrectTex.y);
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
        {1.f,1.f,3.f}, //eye
        {0.f,0.f,0.f}, //centre
        {0.f,1.f,0.f}  //up
    );

    
	FrameBuffer image{kheight,kwidth};
	std::ofstream out_file{"/home/sc2046/Projects/Graphics/CuRa/scenes/05PerspectiveCorrectInterpolation/with-perspective.ppm"};

    //Load model and the associated texture(s).
    const Model head("/home/sc2046/Projects/Graphics/CuRa/assets/models/head.obj");
    const FrameBuffer head_diffuse_map =  ParsePPMTexture("/home/sc2046/Projects/Graphics/CuRa/assets/textures/head_diffuse.ppm");

    const Model floor("/home/sc2046/Projects/Graphics/CuRa/assets/models/floor.obj");
    const FrameBuffer floor_diffuse_map = ParsePPMTexture("/home/sc2046/Projects/Graphics/CuRa/assets/textures/floor_diffuse.ppm"); 

    //Before we implement a proper pipeline object, create some utility containers to store models and their textures
    #include <utility>
    #include <vector>
    using modelPair = std::pair<Model, FrameBuffer>;
    using modelList = std::vector<modelPair>;

    modelList models;
    models.emplace_back(head,head_diffuse_map);
    models.emplace_back(floor,floor_diffuse_map);

    for(const auto& [model, diffuse_map] : models ) {
        //Iterate over each face (triangle) in the model
        for(const auto& face : model.Faces()) {

            std::array<ClippedVertex,3> clippedvertices;

            for(int i =0;i<3;++i) {

                //Get position of vertex in 3D world space and convert to homogeneous coordinates.
                const auto worldpos = model.Vertices()[face.pos_idx[i]]; 
                const auto hworldpos = Vec4f(worldpos,1.f);

                //Transform to camera space by applying view matrix.
                const auto lookat = camera.view;
                const auto hcamerapos = la::mul(lookat, hworldpos);

                //Transform to clip space by applying projection matrix.
                //const auto projection_matrix = OrthographicProjection(-1.f,1.f,-1.f,1.f,-1.f,-5.f);
                const auto projection_matrix = PerspectiveProjection(std::numbers::pi_v<float>/2.f, kaspect_ratio,-0.1f,-5.f);
                
                const auto hclipspacepos = la::mul(projection_matrix,hcamerapos);

                //Clipping (TODO)

                //Transform to NDC by applying perspective divide. (Note this does nothing for an orthographic projection).
                const auto ndcpos = hclipspacepos.xyz() / hclipspacepos.w;

                //Transform to screen space by applying viewport transformation.
                //Keep the z coordinate for depth testing.
                const auto viewpos = Vec3f{(ndcpos.x+1)*kwidth/2.f, (-ndcpos.y+1)*kheight/2.f, ndcpos.z};
                //Also get other attributes from the model...
                const auto texcoord = model.TexCoords()[face.tex_idx[i]];

                clippedvertices[i] = ClippedVertex{viewpos, texcoord, hcamerapos.z};
            }
            //Rasterise & color
            DrawTriangle(clippedvertices[0],clippedvertices[1],clippedvertices[2], image, diffuse_map);
        }
    }

	if(!out_file) {std::cerr<<"Error creating file\n"; return 1;};
	image.WriteColorsPPM(out_file);
}