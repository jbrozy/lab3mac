#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace Object {
typedef struct vec3 {
  float x, y, z;
} Vec3;

typedef struct vec2 {
  float x, y;
} Vec2;

typedef struct face {
  int vert_idx[3], tex_coord_idx[3], normal_idx[3];
} Face;

static int _refid = 0;

typedef struct object {
  int id = ++_refid;
  std::vector<vec3> vertices;
  std::vector<vec2> tex_coords;
  std::vector<vec3> normals;
  std::vector<Face> faces;
} Object;

inline Object parse(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cout << "File not found!" << std::endl;
  }

  Object obj;
  std::string line;

  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string prefix;
    iss >> prefix;

    if (prefix == "v") {
      vec3 vertex;
      iss >> vertex.x >> vertex.y >> vertex.z;
      obj.vertices.push_back(vertex);
    } else if (prefix == "vt") {
      vec2 texCoord;
      iss >> texCoord.x >> texCoord.y;
      obj.tex_coords.push_back(texCoord);
    } else if (prefix == "vn") {
      vec3 normal;
      iss >> normal.x >> normal.y >> normal.z;
      obj.normals.push_back(normal);
    } else if (prefix == "f") {
      Face face;
      for (int i = 0; i < 3; ++i) {
        std::string vertex;
        iss >> vertex;
        sscanf(vertex.c_str(), "%d/%d/%d", &face.vert_idx[i],
               &face.tex_coord_idx[i], &face.normal_idx[i]);

        // OBJ indices are 1-based, convert to 0-based
        face.vert_idx[i]--;
        face.tex_coord_idx[i]--;
        face.normal_idx[i]--;
      }
      obj.faces.push_back(face);
    }
  }
  file.close();
  return obj;
}
} // namespace Object
