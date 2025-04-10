// MainMenu.cpp
#include "MainMenu.h"
#include "Utility.h"
#include <SDL.h>

void MainMenu::initialise() {
    m_game_state.next_scene_id = -1; 
    m_font_texture_id = Utility::load_texture("assets/font1.png");
    m_game_state.player = nullptr;
}

void MainMenu::update(float delta_time) {
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_RETURN]) {
        m_game_state.next_scene_id = 1;
    }
}

void MainMenu::render(ShaderProgram *program) {
    Utility::draw_text(program, m_font_texture_id, "MY AWESOME GAME", 0.5f, 0.05f, glm::vec3(-3.0f, 1.0f, 0));
    Utility::draw_text(program, m_font_texture_id, "Press ENTER to start", 0.35f, 0.05f, glm::vec3(-2.5f, -1.0f, 0));
}
