#pragma once

#include "stdio.h"
#include "stdlib.h"

#include <string.h>

// soll unabhängig von linmath.h sein, daher Redefinition von vec2/3
struct t_vec3 {
  t_vec3(char *buf) // generiere vec3 aus string
  {
    char *next;
    strtok_r(buf, " ", &next);
    x = atof(strtok_r(next, " ", &next));
    y = atof(strtok_r(next, " ", &next));
    z = atof(strtok_r(next, " ", &next));
  }
  t_vec3() { x = y = z = 0; }
  float x, y, z;
};

struct t_vec2 {
  t_vec2(char *buf) // generiere vec2 aus string
  {
    char *next;
    strtok_r(buf, " ", &next);
    u = atof(strtok_r(next, " ", &next));
    v = atof(strtok_r(next, " ", &next));
  }
  float u, v;
};

struct t_triangleIndex {

  t_triangleIndex(char *vert1, char *vert2, char *vert3) {
    char *next;
    ver[0] = atoi(strtok_r(vert1, "/", &next));
    tex[0] = atoi(strtok_r(next, "/", &next));
    norm[0] = atoi(strtok_r(next, "/", &next));

    ver[1] = atoi(strtok_r(vert2, "/", &next));
    tex[1] = atoi(strtok_r(next, "/", &next));
    norm[1] = atoi(strtok_r(next, "/", &next));

    ver[2] = atoi(strtok_r(vert3, "/", &next));
    tex[2] = atoi(strtok_r(next, "/", &next));
    norm[2] = atoi(strtok_r(next, "/", &next));
  }
  int ver[3];
  int tex[3];
  int norm[3];
};

struct myVertexType {
  float x, y, z;
  float xn, yn, zn;
  float u, v;
  myVertexType(t_vec3 vert, t_vec3 norm) {
    x = vert.x;
    y = vert.y;
    z = vert.z;
    xn = norm.x;
    yn = norm.y;
    zn = norm.z;
    u = v = 0;
  }
  myVertexType(t_vec3 vert, t_vec3 norm, t_vec2 uv) {
    x = vert.x;
    y = vert.y;
    z = vert.z;
    xn = norm.x;
    yn = norm.y;
    zn = norm.z;
    u = uv.u;
    v = uv.v;
  }
};

// load BMP from file "filename", pointer to "number" returns the number of
// vertices, returns VertexArray
myVertexType *loadModel(const char *filename, int *number) {
  t_vec3 *vertexCoordinates = NULL;
  t_vec3 *vertexNormals = NULL;
  t_vec2 *vertexTextureUV;
  t_triangleIndex *triangles;
  myVertexType *vertices;
  int numVertices = 0;
  int numNormals = 0;
  int numTexture = 0;
  int numTable = 0;
  int numTriangles = 0;
  FILE *f = fopen(filename, "r");

  if (f == NULL) {
    printf("Objekt Datei %s nicht gefunden!", filename);
    exit(0);

  } else
    printf("Objekt Datei %s gefunden!\n", filename);

  char buf[255];
  while (fgets(buf, 255, f)) {
    if (buf[0] == 'v')
      switch (buf[1]) {
      case (' '):
        numVertices++;
        break;
      case ('t'):
        numTexture++;
        break;
      case ('n'):
        numNormals++;
        break;
      default:
        break;
      }
    if (buf[0] == 'f')
      numTable++;
    //	printf("%s", buf);
  }
  fseek(f, 0, SEEK_SET);
  printf("Gefunden wurden %d Vertexe, %d Normalen, %d Texturekoordinaten und "
         "eine Tabelle mit %d Dreiecken\n",
         numVertices, numNormals, numTexture, numTable);

  vertexCoordinates = (t_vec3 *)malloc(numVertices * sizeof(t_vec3));
  vertexNormals = (t_vec3 *)malloc(numNormals * sizeof(t_vec3));
  vertexTextureUV = (t_vec2 *)malloc(numTexture * sizeof(t_vec2));
  triangles = (t_triangleIndex *)malloc(numTable * sizeof(t_triangleIndex));
  vertices = (myVertexType *)malloc(
      numTable * 3 *
      sizeof(myVertexType)); //*3 because of three vertices per triangle

  char *next;
  char *cont;
  float x;
  float y;
  float z;
  numVertices = 0;
  numNormals = 0;
  numTexture = 0;
  numTable = 0;
  while (fgets(buf, 255, f)) {
    if (buf[0] == 'v')
      switch (buf[1]) {
      case (' '):
        vertexCoordinates[numVertices] = t_vec3(buf);
        numVertices++;
        break;
      case ('t'):
        vertexTextureUV[numTexture] = t_vec2(buf);
        numTexture++;
        break;
      case ('n'):
        vertexNormals[numNormals] = t_vec3(buf);
        numNormals++;
        break;
      default:
        break;
      }
    if (buf[0] == 'f') // one face -- one triangle
    {
      char *next;
      char *vert1;
      char *vert2;
      char *vert3;

      strtok_r(buf, " ", &next); // f away
      vert1 = strtok_r(next, " ", &next);
      vert2 = strtok_r(next, " ", &next);
      vert3 = strtok_r(next, " ", &next);

      triangles[numTable++] = t_triangleIndex(vert1, vert2, vert3);
    }
  }
  // generate vertices
  for (int i = 0; i < numTable; i++) {
    vertices[i * 3] = myVertexType(vertexCoordinates[triangles[i].ver[0] - 1],
                                   vertexNormals[triangles[i].norm[0] - 1],
                                   vertexTextureUV[triangles[i].tex[0] - 1]);
    vertices[i * 3 + 1] =
        myVertexType(vertexCoordinates[triangles[i].ver[1] - 1],
                     vertexNormals[triangles[i].norm[1] - 1],
                     vertexTextureUV[triangles[i].tex[1] - 1]);
    vertices[i * 3 + 2] =
        myVertexType(vertexCoordinates[triangles[i].ver[2] - 1],
                     vertexNormals[triangles[i].norm[2] - 1],
                     vertexTextureUV[triangles[i].tex[2] - 1]);
    //-1 because of index start at 1 in obj-file
  }

  *number = numTable * 3;
  return vertices;
}
