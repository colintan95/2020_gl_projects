# TODO

#### Now
- Figure out how to test the correctness of shader implementations

#### Shader Techniques
- Implement deferred shading
- Implement reflections
- Implement opacity and refraction
- Implement more shadow techniques (e.g. soft shadows)

#### Framework
- Continue building the asset loader - making it work with any .obj file
  - Computing the normals in code when the .obj file doesn't specify them
  - Supporting all illum modes
  - Profile and optimize the loading code - especially for larger models (e.g. Sponza)
- Start writing tests for parts of the code where unit tests are most beneficial
  - Tests are important for refactoring - to make sure I'm not breaking anything when I clean up the code and make the API better.
- Profile the framework code and optimize

#### Future
- Consider how to support other graphics APIs (e.g. Vulkan, DX12)