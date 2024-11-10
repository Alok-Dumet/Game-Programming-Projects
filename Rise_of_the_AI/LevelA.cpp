//* Author: Alok Aenugu
//* Assignment : Rise of the AI
//* Date due : 2024 - 11 - 9, 11 : 59pm
//* I pledge that I have completed this assignment without
//* collaborating with anyone else, in conformance with the
//* NYU School of Engineering Policies and Procedures on
//* Academic Misconduct.

#include "LevelA.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

constexpr char FONTSHEET_FILEPATH[] = "assets/font1.png",
           SPRITESHEET_FILEPATH[] = "assets/DK.png",
           ENEMY1_FILEPATH[]       = "assets/koopa.png",                                              //Created new enemy skins
           ENEMY2_FILEPATH[] = "assets/Mario_Tiles.png",
           ENEMY3_FILEPATH[] = "assets/george_0.png";

GLuint g_font_texture_id = 1;

unsigned int LEVEL_DATA[] =
{
    4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 4,
    4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 4,
    4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 4,
    4, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 3, 4,
    4, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 3, 4,
    4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 4,
    4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 4,
    4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 4
};

LevelA::~LevelA()
{
    delete [] m_game_state.enemies;
    delete    m_game_state.enemy1;
    delete    m_game_state.enemy1;
    delete    m_game_state.enemy1;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelA::initialise()
{
    GLuint map_texture_id = Utility::load_texture("assets/Mario_Tiles.png");
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVEL_DATA, map_texture_id, 1.0f, 5, 1);
    
    GLuint player_texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);

    int player_walking_animation[4][4] =
    {
        { 1, 5, 9, 13 },  // for George to move to the left,
        { 3, 7, 11, 15 }, // for George to move to the right,
        { 2, 6, 10, 14 }, // for George to move upwards,
        { 0, 4, 8, 12 }   // for George to move downwards
    };

    glm::vec3 acceleration = glm::vec3(0.0f, -4.81f, 0.0f);

    m_game_state.player = new Entity(
        player_texture_id,         // texture id
        5.0f,                      // speed
        acceleration,              // acceleration
        10.0f,                      // jumping power
        player_walking_animation,  // animation index sets
        0.0f,                      // animation time
        4,                         // animation frame amount
        0,                         // current animation index
        4,                         // animation column amount
        4,                         // animation row amount
        0.7f,                      // width
        0.7f,                       // height
        PLAYER
    );
    
    m_game_state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));

    // Jumping
    m_game_state.player->set_jumping_power(5.5f);

    g_font_texture_id = Utility::load_texture(FONTSHEET_FILEPATH);
    
    /**
     Enemies' stuff */
    GLuint enemy1_texture_id = Utility::load_texture(ENEMY1_FILEPATH);
    GLuint enemy2_texture_id = Utility::load_texture(ENEMY2_FILEPATH);
    GLuint enemy3_texture_id = Utility::load_texture(ENEMY3_FILEPATH);

    m_game_state.enemies = new Entity[ENEMY_COUNT];


    //m_game_state.enemy1 = new Entity(enemy1_texture_id, 1.0f, 0.5f, 0.8f, ENEMY, GUARD, PATROLLING);
    //m_game_state.enemy1->set_position(glm::vec3(8.0f, 6.0f, 0.0f));
    //m_game_state.enemy1->set_movement(glm::vec3(0.0f));
    //m_game_state.enemy1->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));

    //m_game_state.enemy2 = new Entity(enemy1_texture_id, 1.0f, 0.5f, 0.8f, ENEMY, GUARD, WALKING);
    //m_game_state.enemy2->set_position(glm::vec3(6.0f, 6.0f, 0.0f));
    //m_game_state.enemy2->set_movement(glm::vec3(0.0f));
    //m_game_state.enemy2->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));

    //m_game_state.enemy3 = new Entity(enemy1_texture_id, 1.0f, 0.5f, 0.8f, ENEMY, GUARD, JUMPING);
    //m_game_state.enemy3->set_position(glm::vec3(4.0f, 6.0f, 0.0f));
    //m_game_state.enemy3->set_movement(glm::vec3(0.0f));
    //m_game_state.enemy3->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));

    m_game_state.enemies[0] = Entity(enemy1_texture_id, 1.0f, 0.5f, 0.8f, ENEMY, GUARD, PATROLLING);
    m_game_state.enemies[0].set_position(glm::vec3(8.0f, -5.0f, 0.0f));
    m_game_state.enemies[0].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));

    m_game_state.enemies[1] = Entity(enemy1_texture_id, 1.0f, 0.5f, 0.8f, ENEMY, GUARD, WALKING);
    m_game_state.enemies[1].set_position(glm::vec3(6.0f, -5.0f, 0.0f));
    m_game_state.enemies[1].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[1].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));

    m_game_state.enemies[2] = Entity(enemy1_texture_id, 1.0f, 0.5f, 0.8f, ENEMY, GUARD, JUMPING);
    m_game_state.enemies[2].set_position(glm::vec3(4.0f, -1.0f, 0.0f));
    m_game_state.enemies[2].set_movement(glm::vec3(0.0f));
    m_game_state.enemies[2].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    m_game_state.enemies[2].set_special(true);



    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/DK_CR.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(60.0f);
    
    m_game_state.jump_sfx = Mix_LoadWAV("assets/M_jump2.wav");
}

void LevelA::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);

    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, NULL, m_game_state.map);
    }

    
    /*m_game_state.enemy1->update(delta_time, m_game_state.player, NULL, NULL, m_game_state.map);
    m_game_state.enemy2->update(delta_time, m_game_state.player, NULL, NULL, m_game_state.map);
    m_game_state.enemy3->update(delta_time, m_game_state.player, NULL, NULL, m_game_state.map);*/

}

void LevelA::render(ShaderProgram *g_shader_program)
{

    m_game_state.map->render(g_shader_program);

    for (int i = 0; i < ENEMY_COUNT; i++) {
        m_game_state.enemies[i].render(g_shader_program);
    }



    m_game_state.player->render(g_shader_program);
    //m_game_state.enemy1->render(g_shader_program);
    //m_game_state.enemy2->render(g_shader_program);
    //m_game_state.enemy3->render(g_shader_program);


    glm::vec3 text_pos = glm::vec3(m_game_state.player->get_position());
    text_pos.y += 3;
    text_pos.x -= 2;

    //float x1 = m_game_state.enemies[0].get_position().x;
    //float x2 = m_game_state.enemies[1].get_position().x;
    //float x3 = m_game_state.enemies[2].get_position().x;

    //float y1 = m_game_state.enemies[0].get_position().y;
    //float y2 = m_game_state.enemies[1].get_position().y;
    //float y3 = m_game_state.enemies[2].get_position().y;

    //Utility::draw_text(g_shader_program, g_font_texture_id, "" + std::to_string(x1) + " " + std::to_string(y1), 0.5f, 0.05f,
    //    text_pos);
    //text_pos.y -= 1;
    //Utility::draw_text(g_shader_program, g_font_texture_id, "" + std::to_string(x2) + " " + std::to_string(y2), 0.5f, 0.05f,
    //    text_pos);
    //text_pos.y -= 1;
    //Utility::draw_text(g_shader_program, g_font_texture_id, "" + std::to_string(x3) + " " + std::to_string(y3), 0.5f, 0.05f,
    //    text_pos);

    //text_pos.y += 2;


    float ENEMIES_LEFT = 0;
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        if (m_game_state.enemies[i].get_active())
        {
            ENEMIES_LEFT++;
        }
    }

    if (ENEMIES_LEFT == 0)
    {
        text_pos.y -= 2;
        Utility::draw_text(g_shader_program, g_font_texture_id, "You did it!", 0.5f, 0.05f,
            text_pos);
    }

    if (!m_game_state.player->get_active())
    {
        if (text_pos.y >= 0)
        {
            text_pos.y = m_game_state.player->get_position().y - 1;
        }
        Utility::draw_text(g_shader_program, g_font_texture_id, "You died!", 0.5f, 0.05f,
            text_pos);
    }

}
