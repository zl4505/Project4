/**
* Author: Zhenan Liu
* Assignment: Rise of the AI
* Date due: 2025-4-9, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "MainMenu.h"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.h"
#include "EndScene.h"

constexpr int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

constexpr float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0f;

enum AppStatus { RUNNING, TERMINATED };

// ————— GLOBAL VARIABLES ————— //
Scene *g_current_scene;
MainMenu *g_main_menu;
LevelA   *g_level_a;
LevelB   *g_level_b;
LevelC   *g_level_c;
EndScene *g_end_scene_win;
EndScene *g_end_scene_lose;

SDL_Window* g_display_window;

AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

int g_player_lives = 3;

void switch_to_scene(Scene *scene) {
    g_current_scene = scene;
    g_current_scene->initialise();
    g_view_matrix = glm::mat4(1.0f);
}

GLuint g_font_texture_id;

void initialise();
void process_input();
void update();
void render();
void shutdown();

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Hello!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      640, 480,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    if (context == nullptr)
    {
        shutdown();
    }
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 480);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    glUseProgram(g_shader_program.get_program_id());
    
    // Load a font texture
    g_font_texture_id = Utility::load_texture("assets/font1.png");
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // ————— SCENE SETUP ————— //
    g_main_menu = new MainMenu();
    g_level_a   = new LevelA();
    g_level_b   = new LevelB();
    g_level_c   = new LevelC();
    g_end_scene_win = new EndScene(true);
    g_end_scene_lose = new EndScene(false);

    switch_to_scene(g_main_menu);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    Entity* player = g_current_scene->get_state().player;
    if (player != nullptr) {
        player->set_movement(glm::vec3(0.0f));
    }

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_q:
                        g_app_status = TERMINATED;
                        break;
                        
                    case SDLK_SPACE:
                        if (player != nullptr && player->get_collided_bottom())
                        {
                            player->jump();
                            Mix_PlayChannel(-1, g_current_scene->get_state().jump_sfx, 0);
                        }
                        break;
                        
                    default:
                        break;
                }
                break;

            default:
                break;
        }
    }

    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    if (player != nullptr)
    {
        if (key_state[SDL_SCANCODE_LEFT]) {
            player->move_left();
        }
        else if (key_state[SDL_SCANCODE_RIGHT]) {
            player->move_right();
        }

        if (glm::length(player->get_movement()) > 1.0f)
        {
            player->normalise_movement();
        }
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP) {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
        g_current_scene->update(FIXED_TIMESTEP);
        delta_time -= FIXED_TIMESTEP;

        int next_scene = g_current_scene->get_state().next_scene_id;
        if (next_scene != -1)
        {
            // Scene switching based on next_scene value:
            if (next_scene == 1) {
                switch_to_scene(g_level_a);
            } else if (next_scene == 2) {
                switch_to_scene(g_level_b);
            } else if (next_scene == 3) {
                switch_to_scene(g_level_c);
            } else if (next_scene == 10) {
                // Win condition
                switch_to_scene(g_end_scene_win);
            } else if (next_scene == 20) {
                // Lose condition
                switch_to_scene(g_end_scene_lose);
            }
            g_accumulator = 0.0f;
            break;
        }
    }
    
    g_accumulator = delta_time;

    g_view_matrix = glm::mat4(1.0f);
    Entity* player = g_current_scene->get_state().player;
    if (player != nullptr) {
        if (player->get_position().x > LEVEL1_LEFT_EDGE) {
            g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-player->get_position().x, 3.75f, 0.0f));
        } else {
            g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5.0f, 3.75f, 0.0f));
        }
    }
}

void render()
{

    g_shader_program.set_view_matrix(g_view_matrix);
    glClear(GL_COLOR_BUFFER_BIT);
    g_current_scene->render(&g_shader_program);
    
    glm::mat4 ui_view = glm::mat4(1.0f);
    g_shader_program.set_view_matrix(ui_view);

    std::string livesText = "Lives: " + std::to_string(g_player_lives);

    Utility::draw_text(&g_shader_program, g_font_texture_id, livesText, 0.5f, 0.05f, glm::vec3(-4.8f, 3.5f, 0));

    SDL_GL_SwapWindow(g_display_window);
}


void shutdown()
{
    SDL_Quit();
    delete g_main_menu;
    delete g_level_a;
    delete g_level_b;
    delete g_level_c;
    delete g_end_scene_win;
    delete g_end_scene_lose;
}

int main(int argc, char* argv[])
{
    initialise();
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }
    shutdown();
    return 0;
}
