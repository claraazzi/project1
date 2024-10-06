/**
* Author: Clara El Azzi
* Assignment: Simple 2D Scene
* Date due: 2023-09-20, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

enum AppStatus { RUNNING, TERMINATED };

constexpr int WINDOW_WIDTH  = 640 * 2;
constexpr int WINDOW_HEIGHT = 480 * 2;

constexpr float BG_RED     = 0.9765625f;
constexpr float BG_GREEN   = 0.97265625f;
constexpr float BG_BLUE    = 0.9609375f;
constexpr float BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X      = 0;
constexpr int VIEWPORT_Y      = 0;
constexpr int VIEWPORT_WIDTH  = WINDOW_WIDTH;
constexpr int VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl";
constexpr char F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();

glm::mat4 g_view_matrix,
          g_kimi_matrix,
          g_bob_matrix,
          g_projection_matrix;

float g_previous_ticks = 0.0f;

GLuint g_kimi_texture_id;
GLuint g_bob_texture_id;

float g_rotation_kimi = 0.0f;
float g_rotation_bob = 0.0f;
float g_scale_kimi = 1.0f;  // For dynamic scaling

// Loading textures for Bob and Kimi
GLuint load_texture(const char* file_path) {
    int width, height, number_of_components;
    unsigned char* image = stbi_load(file_path, &width, &height, &number_of_components, STBI_rgb_alpha);
    if (image == NULL) {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);
    return texture_id;
}

void initialise() {
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Assignment", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GL_CreateContext(g_display_window);

    // Set up the viewport
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    g_view_matrix = glm::mat4(1.0f);

    g_shader_program.set_view_matrix(g_view_matrix);
    g_shader_program.set_projection_matrix(g_projection_matrix);

    // Load textures from the textures folder
    g_kimi_texture_id = load_texture("textures/kimi.png");
    g_bob_texture_id = load_texture("textures/bob.png");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            g_app_status = TERMINATED;
        }
    }
}

void update() {
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    // Kimi's rotation and scaling
    g_rotation_kimi += 90.0f * delta_time;
    g_scale_kimi = 1.0f + 0.5f * delta_time;  // Dynamic scaling over time
    g_kimi_matrix = glm::mat4(1.0f);
    g_kimi_matrix = glm::translate(g_kimi_matrix, glm::vec3(2.0f * cosf(g_rotation_kimi), 2.0f * sinf(g_rotation_kimi), 0.0f));  // Circular movement
    g_kimi_matrix = glm::rotate(g_kimi_matrix, glm::radians(g_rotation_kimi), glm::vec3(0.0f, 0.0f, 1.0f));
    g_kimi_matrix = glm::scale(g_kimi_matrix, glm::vec3(g_scale_kimi, g_scale_kimi, 1.0f));

    // Bob's relative movement (circular movement)
    g_rotation_bob += 90.0f * delta_time;
    g_bob_matrix = glm::mat4(1.0f);
    g_bob_matrix = glm::rotate(g_bob_matrix, glm::radians(g_rotation_bob), glm::vec3(0.0f, 0.0f, 1.0f));
    g_bob_matrix = glm::translate(g_bob_matrix, glm::vec3(1.5f * cosf(g_rotation_bob), 1.5f * sinf(g_rotation_bob), 0.0f));
}

void draw_object(glm::mat4 &object_g_model_matrix, GLuint &object_texture_id) {
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // Drawing 2 triangles (6 vertices)
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // Render both objects
    draw_object(g_kimi_matrix, g_kimi_texture_id);
    draw_object(g_bob_matrix, g_bob_texture_id);

    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    initialise();
    while (g_app_status == RUNNING) {
        process_input();
        update();
        render();
    }
    shutdown();
    return 0;
}
//done
