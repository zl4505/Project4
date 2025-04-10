#include "LevelA.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

// Filepaths for assets
constexpr char SPRITESHEET_FILEPATH[] = "assets/betty.png";
constexpr char ENEMY_FILEPATH[]       = "assets/guard.png";

// A 14x8 level layout
unsigned int LEVEL_DATA[] =
{
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    3, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

LevelA::~LevelA()
{
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelA::initialise()
{
    GLuint map_texture_id = Utility::load_texture("assets/tileset.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVEL_DATA, map_texture_id, 1.0f, 4, 1);

    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);
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

    GLuint enemy_texture_id = Utility::load_texture(ENEMY_FILEPATH);
    m_game_state.enemies = new Entity[ENEMY_COUNT];
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        // In LevelA, keep enemy as GUARD type.
        m_game_state.enemies[i] = Entity(enemy_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, GUARD, IDLE);
    }
    m_game_state.enemies[0].set_position(glm::vec3(8.0f, 0.0f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    m_game_state.bgm = Mix_LoadMUS("assets/song18.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(128.0f);
    m_game_state.jump_sfx = Mix_LoadWAV("assets/SFX_Jump_01.wav");
    m_game_state.death_sfx = Mix_LoadWAV("assets/losetrumpet.wav");
    m_game_state.win_sfx = Mix_LoadWAV("assets/Winsound.wav");

    // Reset next_scene_id for no immediate transition.
    m_game_state.next_scene_id = -1;
}

void LevelA::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, 0, m_game_state.map);
    }
    
    // --- Collision Check: if the player touches any enemy, trigger lose ---
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
    
    // Check for falling off the level:
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
    
    // Check for level completion (player reaches end of LevelA)
    if(m_game_state.player->get_position().x > LEVEL_WIDTH * 1.0f)
    {
        m_game_state.next_scene_id = 2; // Transition to LevelB
    }
}

void LevelA::render(ShaderProgram *program)
{
    m_game_state.map->render(program);
    m_game_state.player->render(program);
    for (int i = 0; i < ENEMY_COUNT; i++)
        m_game_state.enemies[i].render(program);
}
