#include <iostream>
#include <cassert>

typedef struct vertex {
    float x, y, z;
} Vertex;

typedef struct object {
    char * name;
    Vertex *vertices;
    size_t vertices_length;
    size_t texture_coords_length;
    size_t normals_length;
    size_t faces_length;
    // size_t line_length;
} Obj;

int main(int argc, char **argv){
    assert(argc > 2 && "No parameters were given.");
    const char * input_obj = argv[1];
    const char * output_c = argv[2];

    fprintf(stdout, "Input: %s\n", input_obj);
    fprintf(stdout, "Output: %s\n", output_c);
}
