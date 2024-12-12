#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "LadeBMP.h"
#include "shader.h"
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

float AMBIENT_FACTOR = 0.3f;
float DIFFUSION_FACTOR = 0.3f;
float SPECULAR_FACTOR = 0.6f;
int SHININESS_FACTOR = 16;

#define PI 3.1415926536
#define WIDTH 1024
#define HEIGHT 768

glm::vec3 lightDir{0, 0, 1};

typedef struct vertex {
  float x, y, z;
  float r, g, b;
  float nx, ny, nz;
  float u, v;

  vertex(vertex other, glm::vec3 norm) {
    x = other.x;
    y = other.y;
    z = other.z;

    r = other.r;
    g = other.g;
    b = other.b;

    nx = norm.x;
    ny = norm.y;
    nz = norm.z;

    u = v = 0.0;
  }
  vertex(vertex other, glm::vec3 norm, glm::vec2 uv) {
    x = other.x;
    y = other.y;
    z = other.z;

    r = other.r;
    g = other.g;
    b = other.b;

    nx = norm.x;
    ny = norm.y;
    nz = norm.z;
    u = uv.x;
    v = uv.y;
  }
  vertex(float x, float y, float z, float r, float g, float b) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->r = r;
    this->g = g;
    this->b = b;
  }
  vertex() { x = y = z = r = g = b = u = v = 0; }
} Vertex;

static double X_POS, Y_POS;

static void cursor_position_callback(GLFWwindow *window, double xpos,
                                     double ypos) {
  X_POS = (xpos / WIDTH * 2) - 1.0;
  Y_POS = (ypos / HEIGHT * 2) - 1.0;
  std::cout << "Position: (" << X_POS << ":" << Y_POS << ")" << std::endl;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (action != 0) // nur beim Hinunterdrücken einer Taste
  {
    if (key == GLFW_KEY_A) // Wenn diese Taste "A" war
    {
      if (mods == 1)           // Wenn Shift zusätzlich gehalten ist
        AMBIENT_FACTOR += .1f; // ambienten Faktor erhöhen
      else
        AMBIENT_FACTOR -= .1f; // ambienten Faktor senken
    }
    if (key == GLFW_KEY_D) // Wenn diese Taste "A" war
    {
      if (mods == 1)             // Wenn Shift zusätzlich gehalten ist
        DIFFUSION_FACTOR += .1f; // ambienten Faktor erhöhen
      else
        DIFFUSION_FACTOR -= .1f; // ambienten Faktor senken
    }
    if (key == GLFW_KEY_S) // Wenn diese Taste "A" war
    {
      if (mods == 1)            // Wenn Shift zusätzlich gehalten ist
        SPECULAR_FACTOR += .1f; // ambienten Faktor erhöhen
      else
        SPECULAR_FACTOR -= .1f; // ambienten Faktor senken
    }
    if (key == GLFW_KEY_G) // Wenn diese Taste "A" war
    {
      if (mods == 1)              // Wenn Shift zusätzlich gehalten ist
        SHININESS_FACTOR += 1.0f; // ambienten Faktor erhöhen
      else
        SHININESS_FACTOR -= 1.0f; // ambienten Faktor senken
    }
  }
}

// Helper function to check OpenGL errors
void checkOpenGLError(const char *stmt, const char *fname, int line) {
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    printf("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
    abort();
  }
}

#define _DEBUG 1
#ifdef _DEBUG
#define GL_CHECK(stmt)                                                         \
  do {                                                                         \
    stmt;                                                                      \
    checkOpenGLError(#stmt, __FILE__, __LINE__);                               \
  } while (0)
#else
#define GL_CHECK(stmt) stmt
#endif

// Helper function to check shader compilation
void checkShaderCompilation(GLuint shader, const char *shaderType) {
  GLint success;
  GLchar infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cerr << shaderType << " shader compilation failed:\n"
              << infoLog << std::endl;
    exit(1);
  }
}

// Helper function to check program linking
void checkProgramLinking(GLuint program) {
  GLint success;
  GLchar infoLog[512];
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, 512, NULL, infoLog);
    std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
    exit(1);
  }
}

int main() {
  GLuint vertex_buffer, vertex_shader, fragment_shader, complete_shader_program;
  GLint matrix_access, ambient, diffusion, specular, shininess, position_access,
      color_access, normal_access, light_dir_access, uv_access;

  std::cout << "Initializing GLFW..." << std::endl;
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return 1;
  }

  Vertex vertices[36]; // Array for vertices
  Vertex cubeEdges[8]{
      /*0*/ Vertex(+1.0f, +1.0f, +1.0f, 1.0f, 0.0f, 0.0f), // red
      /*1*/ Vertex(+1.0f, +1.0f, -1.0f, 0.0f, 1.0f, 0.0f), // green
      /*2*/ Vertex(+1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f), // blue
      /*3*/ Vertex(+1.0f, -1.0f, +1.0f, 1.0f, 0.0f, 1.0f), // magenta
      /*4*/ Vertex(-1.0f, +1.0f, +1.0f, 0.0f, 1.0f, 1.0f), // cyan
      /*5*/ Vertex(-1.0f, -1.0f, +1.0f, 1.0f, 1.0f, 0.0f), // yellow
      /*6*/ Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f), // white
      /*7*/ Vertex(-1.0f, +1.0f, -1.0f, 0.0f, 0.0f, 0.0f)  // black
  };

  // Scale cube coordinates
  cubeEdges[0] = vertex(vertex(+1.0f, +1.0f, +1.0f, 1.0f, 0.0f, 0.0f),
                        glm::vec3(1.0, 0.0, 0.0));
  for (int i = 0; i < 8; i++) {
    cubeEdges[i].x *= 0.5;
    cubeEdges[i].y *= 0.5;
    cubeEdges[i].z *= 0.5;
  }

  glm::vec3 leftside{1, 0, 0};
  glm::vec3 rightside{-1, 0, 0};
  glm::vec3 front{0, 0, 1};
  glm::vec3 top{0, 1, 0};
  glm::vec3 bottom{0, -1, 0};
  glm::vec3 back{0, 0, -1};

  // Left side
  vertices[0] = vertex(cubeEdges[2], leftside);
  vertices[1] = vertex(cubeEdges[1], leftside);
  vertices[2] = vertex(cubeEdges[0], leftside);

  vertices[3] = vertex(cubeEdges[3], leftside);
  vertices[4] = vertex(cubeEdges[2], leftside);
  vertices[5] = vertex(cubeEdges[0], leftside);

  // Front
  vertices[6] = vertex(cubeEdges[4], front);
  vertices[7] = vertex(cubeEdges[0], front);
  vertices[8] = vertex(cubeEdges[3], front);

  vertices[9] = vertex(cubeEdges[3], front);
  vertices[10] = vertex(cubeEdges[4], front);
  vertices[11] = vertex(cubeEdges[5], front);

  // top side
  vertices[12] = vertex(cubeEdges[4], top);
  vertices[13] = vertex(cubeEdges[0], top);
  vertices[14] = vertex(cubeEdges[1], top);

  vertices[15] = vertex(cubeEdges[4], top);
  vertices[16] = vertex(cubeEdges[7], top);
  vertices[17] = vertex(cubeEdges[1], top);

  // back
  vertices[18] = vertex(cubeEdges[2], back);
  vertices[19] = vertex(cubeEdges[6], back);
  vertices[20] = vertex(cubeEdges[1], back);

  vertices[21] = vertex(cubeEdges[6], back);
  vertices[22] = vertex(cubeEdges[7], back);
  vertices[23] = vertex(cubeEdges[1], back);

  // right side
  vertices[24] = vertex(cubeEdges[6], rightside);
  vertices[25] = vertex(cubeEdges[5], rightside);
  vertices[26] = vertex(cubeEdges[4], rightside);

  vertices[27] = vertex(cubeEdges[6], rightside);
  vertices[28] = vertex(cubeEdges[4], rightside);
  vertices[29] = vertex(cubeEdges[7], rightside);

  // bottom side
  vertices[30] = vertex(cubeEdges[5], bottom);
  vertices[31] = vertex(cubeEdges[3], bottom);
  vertices[32] = vertex(cubeEdges[2], bottom);

  vertices[33] = vertex(cubeEdges[5], bottom);
  vertices[34] = vertex(cubeEdges[2], bottom);
  vertices[35] = vertex(cubeEdges[6], bottom);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Debug", NULL, NULL);
  if (window == NULL) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);
  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    return 1;
  }

  std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

  // Create and compile shaders
  const char *vertexShaderSource = read_from_file("assets/vertexShader.glsl");

  const char *fragmentShaderSource =
      read_from_file("assets/fragmentShader.glsl");

  complete_shader_program = glCreateProgram();
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  GL_CHECK(glShaderSource(vertex_shader, 1, &vertexShaderSource, NULL));
  GL_CHECK(glCompileShader(vertex_shader));
  checkShaderCompilation(vertex_shader, "Vertex");

  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  GL_CHECK(glShaderSource(fragment_shader, 1, &fragmentShaderSource, NULL));
  GL_CHECK(glCompileShader(fragment_shader));
  checkShaderCompilation(fragment_shader, "Fragment");

  // Linken des Shader-Progamms
  glAttachShader(complete_shader_program, vertex_shader);
  glAttachShader(complete_shader_program, fragment_shader);
  glLinkProgram(complete_shader_program);

  GL_CHECK(glDeleteShader(vertex_shader));
  GL_CHECK(glDeleteShader(fragment_shader));

  // Aufbau und Cbermittlung des Vertex-Buffers
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Fügen Sie vor dem Buffer-Setup hinzu:
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // Get shader locations
  matrix_access = glGetUniformLocation(complete_shader_program, "matrix");
  ambient = glGetUniformLocation(complete_shader_program, "ambientFactor");
  diffusion = glGetUniformLocation(complete_shader_program, "diffuseFactor");
  specular = glGetUniformLocation(complete_shader_program, "specularFactor");
  shininess = glGetUniformLocation(complete_shader_program, "shininessFactor");
  light_dir_access = glGetUniformLocation(complete_shader_program, "lightPos");

  // Zugriff auf Position und Farbe innerhalb des Vertex-Buffers
  position_access = glGetAttribLocation(complete_shader_program, "position");
  color_access = glGetAttribLocation(complete_shader_program, "color");
  normal_access = glGetAttribLocation(complete_shader_program, "normals");
  uv_access = glGetAttribLocation(complete_shader_program, "texCoords");

  GLuint texture;
  int picture_width, picture_height;
  auto const pictureData =
      loadBMP24("assets/Cube.bmp", &picture_width, &picture_height);

  std::cout << "Width: " << picture_width << " Height: " << picture_height
            << std::endl;

  glGenTextures(1, &texture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, picture_width, picture_height, 0,
               GL_BGR, GL_UNSIGNED_BYTE, pictureData);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  delete[] pictureData;
  glGenerateMipmap(GL_TEXTURE_2D);
  GLuint textureId =
      glGetUniformLocation(complete_shader_program, "textureSampler");

  // Die Daten fC<r Position und Farbe werden dem Shader mitgeteilt
  glEnableVertexAttribArray(position_access);
  glVertexAttribPointer(position_access, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                        (void *)0);

  glEnableVertexAttribArray(color_access);
  glVertexAttribPointer(color_access, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                        (void *)(sizeof(float) * 3));

  glEnableVertexAttribArray(normal_access);
  glVertexAttribPointer(normal_access, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                        (void *)(sizeof(float) * 6));

  glEnableVertexAttribArray(uv_access);
  glVertexAttribPointer(uv_access, 2, GL_FLOAT, GL_FALSE, sizeof(vertex),
                        (void *)(sizeof(float) * 9));

  // Aufbau und Cbermittlung des Vertex-Buffers
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Die Daten fC<r Position und Farbe werden dem Shader mitgeteilt
  glEnableVertexAttribArray(position_access);
  glVertexAttribPointer(position_access, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                        (void *)0);

  glEnableVertexAttribArray(color_access);
  glVertexAttribPointer(color_access, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                        (void *)(sizeof(float) * 3));

  glEnableVertexAttribArray(normal_access);
  glVertexAttribPointer(normal_access, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                        (void *)(sizeof(float) * 6));

  // Z-Achse positiv defineren
  glClearDepth(0);
  glDepthFunc(GL_GREATER); // grC6Cere Z-Werte C<berschreiben kleinere
  glEnable(GL_DEPTH_TEST); // Verdeckung der Dreiecke untereinander aktivieren

  glfwSetKeyCallback(window,
                     key_callback); // Key-Callback setzten (Funktion oben)
  using clock = std::chrono::high_resolution_clock;
  auto previousTime = clock::now();
  glfwSetCursorPosCallback(window, cursor_position_callback);

  glUseProgram(complete_shader_program);
  // Main render loop
  while (!glfwWindowShouldClose(window)) {
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    auto currentTime = clock::now();
    std::chrono::duration<float> deltaTime = currentTime - previousTime;
    previousTime = currentTime;

    const auto dt = deltaTime.count();
    glm::mat4 matrix{1.0};
    matrix = glm::rotate(
        matrix, (float)glfwGetTime(),
        glm::vec3(0, 1, 1)); // where x, y, z is axis of rotation (e.g. 0 1 0)

    GL_CHECK(glUniform1f(ambient,
                         AMBIENT_FACTOR)); // Der ambiente Faktor wird in das
                                           // Shader-Programm C<bertragen
    GL_CHECK(glUniform1f(diffusion,
                         DIFFUSION_FACTOR)); // Der ambiente Faktor wird in das
                                             // Shader-Programm C<bertragen
    GL_CHECK(glUniform1f(specular,
                         SPECULAR_FACTOR)); // Der ambiente Faktor wird in das
                                            // Shader-Programm C<bertragen
    GL_CHECK(glUniform1i(shininess, SHININESS_FACTOR));
    GL_CHECK(
        glUniformMatrix4fv(matrix_access, 1, GL_FALSE, glm::value_ptr(matrix)));
    glUniform3f(light_dir_access, X_POS, Y_POS, (1 - X_POS) / (1 - Y_POS));

    // Textur aktivieren und binden
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(textureId, 0);
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 36));

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Cleanup
  GL_CHECK(glDeleteBuffers(1, &vertex_buffer));
  GL_CHECK(glDeleteProgram(complete_shader_program));

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
