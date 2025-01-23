#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// Texture sampler
uniform sampler2D ourTexture;
uniform float visibility;

float random(vec2 coord) {
    return fract(sin(dot(coord.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

// Makes it render in a checkerboard pattern. 0 = no pixels visible. 0.5 = 50% of pixels visible randomly. 1 = all pixels visible
void main()
{
    vec4 texColor = texture(ourTexture, TexCoord);

    float randValue = random(TexCoord * 100.0);

    if (randValue > visibility) {
        discard;
    } else {
        FragColor = texColor;
    }
}
