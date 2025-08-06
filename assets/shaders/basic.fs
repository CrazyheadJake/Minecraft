#version 330

// Input from vertex shader
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Uniform values (from CPU)
uniform sampler2D texture0; // The main texture
uniform float time; // A time uniform for animation

float lightIntensity;
vec3 lightDirection;
vec3 lightColor;


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
    float lightMin = 0.2;

    vec4 texelColor = texture(texture0, fragTexCoord);
    vec4 offset = vec4(-0.5f, -0.5f, -0.5f, 0.0);
    
    // Simple color tinting based on time for a basic effect
    if (texelColor.a < 0.5) // If alpha is less than 0.5 (half transparent)
    {
        discard; // Stop processing this fragment immediately
    }

    lightIntensity = 1.0f;
    lightDirection = normalize(vec3(0, 1.0, 0));
    lightIntensity *= remapAndClamp(dot(lightDirection, fragNormal), -1, 1, 0.5, 1);
    // lightColor = vec3(125/255.0f, 125/255.0f, 162/255.0f);
    lightColor = vec3(1, 1, 1);

    vec3 mappedColor = remapAndClamp(lightColor, vec3(0.0), vec3(1.0), vec3(lightMin), vec3(lightMin) + vec3(lightIntensity) * fragColor.xyz);
    // mappedColor = remapAndClamp(lightColor, vec3(lightMin), vec3(lightIntensity), vec3(lightMin), vec3(lightMin) + vec3(lightIntensity) * fragColor.xyz);
    texelColor.xyz = texelColor.xyz * mappedColor;

    vec4 tintedColor = texelColor;

    // float saturation = mix(fragColor.x, 1.0, 0.35);
    // float luminance = (tintedColor.x * 0.299 + tintedColor.y * 0.587 + tintedColor.z * 0.114) / 3.0;
    // vec4 grayColor = vec4(luminance, luminance, luminance, texelColor.a);
    // tintedColor = mix(grayColor, tintedColor, saturation);
    finalColor = tintedColor;
}