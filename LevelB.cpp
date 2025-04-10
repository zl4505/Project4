#include "LevelB.h"
#include "Utility.h"

#define LEVELB_WIDTH 20
#define LEVELB_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH_B[] = "assets/betty.png";
constexpr char ENEMY_FILEPATH_B[]       = "assets/fly.png";

unsigned int LEVELB_DATA[] =
{
    // Rows 1–5: empty rows with border (tile 3)
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    // Row 6: a platform segment in the middle
    3, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    // Row 7: ground row with more tiles on the left
    3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0,
    // Row 8: full ground row
    3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

LevelB::~LevelB()
{
    delete [] m_game_state.enemies;
    delete m_game_state.player;
    delete m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelB::initialise()
{
    GLuint map_texture_id = Utility::load_texture("assets/tileset.png");
    m_game_state.map = new Map(LEVELB_WIDTH, LEVELB_HEIGHT, LEVELB_DATA, map_texture_id, 1.0f, 4, 1);

    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH_B);
    int player_walking_animation[4][4] =
    {
        { 1, 5, 9, 13 },
        { 3, 7, 11, 15 },
        { 2, 6, 10, 14 },
        { 0, 4, 8, 12 }
    };
    glm::vec3 acceleration = glm::vec3(0.0f, -4.81f, 0.0f);
    m_game_state.player = new Entity(
        player_texture_id,
        5.0f,
        acceleration,
        5.0f,
        player_walking_animation,
        0.0f,
        4,
        0,
        4,
        4,
        1.0f,
        1.0f,
        PLAYER
    );
    m_game_state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));
    m_game_state.player->set_jumping_power(4.0f);

    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH_B);
    m_game_state.enemies = new Entity[ENEMY_COUNT];
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        // Create enemy with default AI type GUARD
        m_game_state.enemies[i] = Entity(enemy_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, GUARD, IDLE);
    }
    // Change enemy 0 to be of type FLYER to cover the third AI type:
    m_game_state.enemies[0].set_ai_type(FLYER);
    m_game_state.enemies[0].set_position(glm::vec3(8.0f, 0.0f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));

    if (ENEMY_COUNT > 1)
    {
        m_game_state.enemies[1].set_position(glm::vec3(12.0f, 0.0f, 0.0f));
        m_game_state.enemies[1].set_movement(glm::vec3(0.0f));
        m_game_state.enemies[1].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    }

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    m_game_state.bgm = Mix_LoadMUS("assets/song18.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(128.0f);
    m_game_state.jump_sfx = Mix_LoadWAV("assets/SFX_Jump_01.wav");
    m_game_state.death_sfx = Mix_LoadWAV("assets/losetrumpet.wav");
    m_game_state.win_sfx = Mix_LoadWAV("assets/Winsound.wav");

    m_game_state.next_scene_id = -1;
}

void LevelB::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);
    }
    
    // --- Collision Check: if player touches any enemy, trigger lose ---
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        if (m_game_state.player->check_collision(&m_game_state.enemies[i]))
        {
            g_player_lives--;
            if(g_player_lives <= 0)
            {
                m_game_state.next_scene_id = 20;
            }
            else
            {
                m_game_state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));
            }
            return;
        }
    }
    
    // Check for falling off:
    if(m_game_state.player->get_position().y < -10.0f)
    {
        g_player_lives--;
        if(g_player_lives <= 0)
        {
            m_game_state.next_scene_id = 20;
        }
        else
        {
            m_game_state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));
        }
    }

    if(m_game_state.player->get_position().x > LEVELB_WIDTH * 1.0f)
    {
        m_game_state.next_scene_id = 3;
    }
}

void LevelB::render(ShaderProgram *program)
{
    m_game_state.map->render(program);
    m_game_state.player->render(program);
    for (int i = 0; i < ENEMY_COUNT; i++)
        m_game_state.enemies[i].render(program);
}
