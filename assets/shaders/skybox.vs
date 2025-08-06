#version 330

// Input vertex attributes (from CPU)
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;

// Output to fragment shader
out vec2 fragTexCoord;
out vec3 fragNormal;
out vec3 fragPosition;

// Uniform values (from CPU)
uniform mat4 mvp; // Model-View-Projection matrix

void main()
{
    fragTexCoord = vertexTexCoord;
    gl_Position = mvp * vec4(vertexPosition, 1.0); // Transform vertex position
    fragPosition = vertexPosition;
    fragNormal = vertexNormal;
}