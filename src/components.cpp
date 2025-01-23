#include "components.hpp"
#include "render_system.hpp" // for gl_has_errors

#define STB_IMAGE_IMPLEMENTATION
#include "../ext/stb_image/stb_image.h"

// stlib
#include <iostream>

// Very, VERY simple OBJ loader from https://github.com/opengl-tutorials/ogl tutorial 7
// (modified to also read vertex color and omit uv and normals)
bool Mesh::loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices,
    std::vector<uint16_t>& out_vertex_indices, std::vector<Edge>& out_edges, vec2& out_size)
{
    // disable warnings about fscanf and fopen on Windows
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

    printf("Loading OBJ file %s...\n", obj_path.c_str());
    std::vector<uint16_t> out_normal_indices;

    // Set to keep track of unique edges (using pair with smaller index first to avoid duplicates)
    std::set<std::pair<uint16_t, uint16_t>> edge_set;

    FILE* file = fopen(obj_path.c_str(), "r");
    if (file == NULL) {
        std::cerr << "Impossible to open the file ! Are you in the right path ?" << std::endl;
        return false;
    }

    while (1) {
        char lineHeader[128];
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break;

        if (strcmp(lineHeader, "v") == 0) {
            ColoredVertex vertex;
            int matches = fscanf(file, "%f %f %f %f %f %f\n", &vertex.position.x, &vertex.position.y, &vertex.position.z,
                &vertex.color.x, &vertex.color.y, &vertex.color.z);
            if (matches == 3)
                vertex.color = { 1,1,1 };
            out_vertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            unsigned int vertexIndex[3], normalIndex[3];

            int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0],
                &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);

            if (matches != 6) {
                std::cerr << "File can't be read by this parser. Try exporting with different options." << std::endl;
                fclose(file);
                return false;
            }

            // Add vertex indices for the face
            for (int i = 0; i < 3; i++) {
                out_vertex_indices.push_back((uint16_t)vertexIndex[i] - 1);
                out_normal_indices.push_back((uint16_t)normalIndex[i] - 1);
            }

            for (int i = 0; i < 3; i++) {
                uint16_t v1 = (uint16_t)vertexIndex[i] - 1;
                uint16_t v2 = (uint16_t)vertexIndex[(i + 1) % 3] - 1;
                edge_set.insert(std::minmax(v1, v2));
            }
        }
        else {
            // Probably a comment, eat up the rest of the line
            char stupidBuffer[1000];
            fgets(stupidBuffer, 1000, file);
        }
    }
    fclose(file);

    out_edges.assign(edge_set.begin(), edge_set.end());

    // Compute bounds of the mesh
    vec3 max_position = { -99999,-99999,-99999 };
    vec3 min_position = { 99999,99999,99999 };
    for (ColoredVertex& pos : out_vertices)
    {
        max_position = glm::max(max_position, pos.position);
        min_position = glm::min(min_position, pos.position);
    }
    if (abs(max_position.z - min_position.z) < 0.001)
        max_position.z = min_position.z + 1;

    vec3 size3d = max_position - min_position;
    out_size = size3d;

    // Normalize mesh to range -0.5 ... 0.5
    for (ColoredVertex& pos : out_vertices)
        pos.position = ((pos.position - min_position) / size3d) - vec3(0.5f, 0.5f, 0.5f);

    return true;
}
