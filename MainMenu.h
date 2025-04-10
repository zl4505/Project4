// MainMenu.h
#pragma once
#include "Scene.h"

class MainMenu : public Scene {
public:
    GLuint m_font_texture_id;

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
