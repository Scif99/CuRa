
struct RenderPipeline {
public:

public:


    //Run vertex shader
    /*
        - Vertex shader should apply MVP transforms at minimum
        - Must return at least a clip-space position
    */


    //Primitive assembly
    /*
        - Face Culling occurs here
    */

   // Primitive clipping
   /*
        - Use w coord to clip.
        - If some edges are outside view volume, generates new vertices on boundary (this can result in more triangles). //NOTE: what happnes to attributes of these new vertices?
        - If all edges are outside view volume, triangle is culled.
   */

   //Perspective Divide

   //Viewport Transform

   //Rasterization

   //Fragment Shader
public:
    //Rasterizer
    //VertexShader = default;
    //FragmentShader = default;
    //RenderProps m_props;


};
