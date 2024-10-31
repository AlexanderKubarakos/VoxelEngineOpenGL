#version 460 core

layout (std430, binding = 0) buffer ExtraData {
    ivec4 data[];
} extraChunkData;

layout(std430, binding = 1) buffer Data {
    uint data[];
} packedData;

out vec3 vertPos;
flat out uint type;
out vec3 normal;

uniform mat4 MVP;
uniform mat4 MV;

float WorldScaler = 1;

const vec3 normals[6] = {
    vec3(0,1,0),
    vec3(0,-1,0),
    vec3(0,0,1),
    vec3(0,0,-1),
    vec3(1,0,0),
    vec3(-1,0,0)
};

// Offsets for our vertices drawing this face
const vec3 facePositions[4] = {
    vec3(0.0f, 0.0f, 1.0f),
    vec3(1.0f, 0.0f, 0.0f),
    vec3(1.0f, 0.0f, 1.0f),
    vec3(0.0f, 0.0f, 0.0f)
};

// Winding order to access the face positions
const int indices[6] = { 0, 1, 2, 1, 0, 3 };

void main()
{
    const int index = gl_VertexID / 6;
    const uint data = packedData.data[index];
    const int currVertexID = gl_VertexID % 6;

    uint x = data & 63;
    uint y = (data >> 6) & 63;
    uint z = (data >> 12) & 63;
    uint lengthX = ((data >> 18) & 63);
    uint lengthY = ((data >> 24) & 63);
    type = ((data >> 30) & 63);

    vec3 position = facePositions[indices[currVertexID]];
    
    // Do scalling
    position.z *= lengthY;
    position.x *= lengthX;

    // Do rotation
    switch (extraChunkData.data[gl_DrawID].w)
    {
    case 0: 
        position.y++;
        break;
    case 1:
        position = position;
        break;
    case 2:
        position.yz = position.zy;
        position.z++;
        break;
    case 3:
        position.yz = position.zy;
        break;
    case 4:
        position.yx = position.xy;
        position.x++;
        break;
    case 5:
        position.yx = position.xy;
        break;
    }
        
    // Offset positions
    position += vec3(x, y, z);
    position = (position + extraChunkData.data[gl_DrawID].xyz * 32) * WorldScaler;

    // Set output data
    normal = normals[extraChunkData.data[gl_DrawID].w];
    gl_Position = MVP * vec4(position, 1.0);
    vec4 viewSpace = MV * vec4(position, 1.0);
    vertPos = vec3(viewSpace) / viewSpace.w;
}