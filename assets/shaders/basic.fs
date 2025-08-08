#version 330
#extension GL_ARB_explicit_uniform_location : enable

// Input from vertex shader
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

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

float lightCycle(float t) {
    if (t < 0.5)
        return smoothstep(0.2, 0.3, t);
    return 1 - smoothstep(0.7, 0.8, t);
}

void main()
{
    float lightMin = 0.2;
    
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec4 offset = vec4(-0.5f, -0.5f, -0.5f, 0.0);
    
    // Simple color tinting based on time for a basic effect
    if (texelColor.a < 0.5) // If alpha is less than 0.5 (half transparent)
    {
        discard; // Stop processing this fragment immediately
    }

    vec3 lightDirection = normalize(vec3(0, 1.0, 0));
    float lightIntensity = remapAndClamp(dot(lightDirection, fragNormal), -1, 1, 0.5, 1);
    vec3 nightColor = vec3(105/255.0f, 105/255.0f, 162/255.0f);
    vec3 dayColor = vec3(1, 1, 1);
    vec3 lightColor = mix(nightColor, dayColor, lightCycle(time));

    vec3 mappedColor = remapAndClamp(lightColor, vec3(0.0), vec3(1.0), vec3(lightMin), vec3(lightMin) + vec3(lightIntensity) * fragColor.xyz * fragColor.xyz);
    texelColor.xyz = texelColor.xyz * mappedColor;
    finalColor = texelColor;
}