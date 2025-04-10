#include "EndScene.h"
#include "Utility.h"
#include <SDL.h>
#include <SDL_mixer.h>

extern int g_player_lives;

static Mix_Chunk *g_winSfx = nullptr;
static Mix_Chunk *g_loseSfx = nullptr;

EndScene::EndScene(bool win) : m_win(win) { }

void EndScene::initialise() {
    Mix_HaltMusic();
    m_game_state.next_scene_id = -1;
    m_font_texture_id = Utility::load_texture("assets/font1.png");

    if (!g_winSfx)
        g_winSfx = Mix_LoadWAV("assets/Winsound.wav");
    if (!g_loseSfx)
        g_loseSfx = Mix_LoadWAV("assets/losetrumpet.wav");

    // Play the appropriate sound effect for win or lose
    if (m_win) {
        Mix_PlayChannel(-1, g_winSfx, 0);
    } else {
        Mix_PlayChannel(-1, g_loseSfx, 0);
    }
}

void EndScene::update(float delta_time) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_KEYDOWN) {
            // Optionally: restart game or exit.
        }
    }
}

void EndScene::render(ShaderProgram *program) {
    std::string message = m_win ? "You Win" : "You Lose";
    Utility::draw_text(program, m_font_texture_id, message, 1.0f, 0.05f, glm::vec3(-2.5f, 0.0f, 0));
}
