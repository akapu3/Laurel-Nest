#version 330 core

in vec2 vUV;

out vec4 FragColor;

uniform float keyframe;
uniform sampler2D screenTexture;
#define GRID_SIZE 10

float rand(vec2 coord) {
    return fract(sin(dot(coord.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec2 uv = vUV; // [0, 1]

    vec2 gridPos = uv * float(GRID_SIZE); // [0, GRID_SIZE]
    ivec2 cell = ivec2(floor(gridPos));

    float minDist = 1.0;
    float secondMinDist = 1.0;

    /*
    Voronoi algorithm:
    For each cell:
    For each neighbor:
    Generate random seed point in that cell.
    Calculate minimum distance and second minimum distance to that seed point from current location.
    */
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            ivec2 neighbor = cell + ivec2(i, j);
            neighbor = ivec2(neighbor.x % GRID_SIZE, neighbor.y % GRID_SIZE); // [0, GRID_SIZE]
            vec2 randOffset = vec2(rand(neighbor + 0.02), rand(neighbor + 0.01)); // [0, 1]

            //               [0, GRID_SIZE], [0, 1]
            vec2 seedPoint = (neighbor + randOffset) / float(GRID_SIZE); // [0, 1]

            float dist = distance(uv, seedPoint);
            if (dist < minDist) {
                secondMinDist = minDist;
                minDist = dist;
            } else if (dist < secondMinDist) {
                secondMinDist = dist;
            }
        }
    }

    float lineIntensity = smoothstep(0.0, 0.005 * keyframe , secondMinDist - minDist);
    float normalizedDist = length(uv - vec2(0.5)) / length(vec2(0.5));
    float visibility = 1;
    float alpha = (1 - lineIntensity) * visibility;

    vec3 currentColor = texture(screenTexture, uv).rgb;
    vec3 lineColor = 1.0 - currentColor;

    FragColor = vec4(lineColor, alpha);
}
