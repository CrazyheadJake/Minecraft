#version 330
#extension GL_ARB_explicit_uniform_location : enable

// Input from vertex shader
in vec2 fragTexCoord;

// Uniform values (from CPU)
uniform sampler2D texture0; // The main texture

// Output fragment color
out vec4 finalColor;

vec3 remapAndClamp(vec3 value, vec3 oldMin, vec3 oldMax, vec3 newMin, vec3 newMax)
{
    vec3 newValue = newMin + (value - oldMin) * (newMax - newMin) / (oldMax - oldMin);
    return clamp(newValue, newMin, newMax);
}
float remapAndClamp(float value, float oldMin, float oldMax, float newMin, float newMax)
{
    float newValue = newMin + (value - oldMin) * (newMax - newMin) / (oldMax - oldMin);
    return clamp(newValue, newMin, newMax);
}

void main()
{
    // Change the transparency depending on how close the color is to black
    vec4 texelColor = texture(texture0, fragTexCoord);
    float avgBlack = (texelColor.x + texelColor.y + texelColor.z) / 3.0;
    if (texelColor.x < 0.16)
        texelColor.xyz *= 8;
    texelColor.a = smoothstep(0.05, 0.2, avgBlack);

    finalColor = texelColor;
}