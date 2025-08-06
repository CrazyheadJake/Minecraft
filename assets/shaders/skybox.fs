#version 330
#extension GL_ARB_explicit_uniform_location : enable

// Input from vertex shader
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragPosition;

// Uniform values (from CPU)
layout(location = 0) uniform float time; // A time uniform for animation
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

float sinusoidal(float t) {
    return (-cos(t * 2 * 3.14159265) + 1) / 2.0;
}

void main()
{
    // Assuming the skybox cube's vertices range from, -1000.0 to 1000.0
    vec3 oldMin = vec3(-50.0);
    vec3 oldMax = vec3(50.0);
    vec3 newMin = vec3(0.0);
    vec3 newMax = vec3(1.0);
    vec3 position = remapAndClamp(fragPosition, oldMin, oldMax, newMin, newMax);
    vec4 topColorDay = vec4(123.0/255.0, 166.0/255.0, 255.0/255.0, 1);
    vec4 topColorNight = vec4(0, 0, 0, 1);
    vec4 bottomColorDay = vec4(185.0/255.0, 212.0/255.0, 255.0/255.0, 1);
    vec4 bottomColorNight = vec4(10.0/255.0, 12.0/255.0, 20.0/255.0, 1);
    vec4 topColor = mix(topColorNight, topColorDay, sinusoidal(time));
    vec4 bottomColor = mix(bottomColorNight, bottomColorDay, sinusoidal(time));

    finalColor = mix(bottomColor, topColor, position.y);
}