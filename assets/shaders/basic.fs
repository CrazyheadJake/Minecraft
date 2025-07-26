#version 330

// Input from vertex shader
in vec2 fragTexCoord;
in vec4 fragColor;

// Uniform values (from CPU)
uniform sampler2D texture0; // The main texture
uniform float time; // A time uniform for animation

// Output fragment color
out vec4 finalColor;

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord);
    
    // Simple color tinting based on time for a basic effect
    // vec4 tintedColor = texelColor * fragColor * abs(sin(time));
    if (texelColor.a < 0.5) // If alpha is less than 0.5 (half transparent)
    {
        discard; // Stop processing this fragment immediately
    }
    finalColor = texelColor;
}