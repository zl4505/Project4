#pragma once
#include "Scene.h"

class EndScene : public Scene {
public:
    bool m_win; // true for win; false for lose
    GLuint m_font_texture_id;
    // Constructor: pass true for win, false for lose.
    EndScene(bool win);
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
