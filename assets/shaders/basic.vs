#version 330

// Input vertex attributes (from CPU)
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

// Output to fragment shader
out vec2 fragTexCoord;
out vec4 fragColor;

// Uniform values (from CPU)
uniform mat4 mvp; // Model-View-Projection matrix

void main()
{
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    gl_Position = mvp * vec4(vertexPosition, 1.0); // Transform vertex position
}