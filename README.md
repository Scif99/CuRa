

TODO
- Constexpr the things
- Clipping for primitives that are partially outside view volume
- Solve issue of shared edges
- Perspective-correct interpolation 
    - Perspective projection preserves lines but not distances
    - Interpolating over the vertices in screen space is not the same as interpolating in 3d space
    - In the image, notice the midpoint between a0 and a1 in 3D does not map to the midpoint of the projections of a0 and a1 in screen space
    - Intuitively, its because we are ignoring depth. 

- Parallelise
    - Get rid of triangle struct