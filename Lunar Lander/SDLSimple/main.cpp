#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 11
#define MOUNTAIN_COUNT 6

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include <cstdlib>
#include "Entity.h"

// ––––– STRUCTS AND ENUMS ––––– //
struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* mountain;
};

// ––––– CONSTANTS ––––– //
constexpr int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

constexpr float BG_RED     = 0.0f,
            BG_BLUE    = 0.0f,
            BG_GREEN   = 0.0f,
            BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

void draw_text(ShaderProgram* shader_program, GLuint font_texture_id, std::string text,
    float font_size, float spacing, glm::vec3 position);

constexpr char SPRITESHEET_FILEPATH[] = "assets/Samus_Sprites.png";
constexpr char PLATFORM_FILEPATH[]    = "assets/Samus_Tile.png";
constexpr char MOUNTAIN_FILEPATH[] = "assets/Metal.png";
constexpr char FONTSHEET_FILEPATH[] = "assets/font1.png";
constexpr int FONTBANK_SIZE = 16;

constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL  = 0;
constexpr GLint TEXTURE_BORDER   = 0;

constexpr int CD_QUAL_FREQ    = 44100,
          AUDIO_CHAN_AMT  = 2,     // stereo
          AUDIO_BUFF_SIZE = 4096;

constexpr char BGM_FILEPATH[] = "assets/Brinstar.mp3",
           SFX_FILEPATH[] = "assets/bounce.wav";

constexpr int PLAY_ONCE = 0,    // play once, loop never
          NEXT_CHNL = -1,   // next available channel
          ALL_SFX_CHNL = -1;


Mix_Music *g_music;
Mix_Chunk *g_boost_sfx;

// ––––– GLOBAL VARIABLES ––––– //
GameState g_state;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

bool gameover = false;
bool won = false;
bool lost = false;

//TEXTURE
GLuint g_font_texture_id;

//Fuel
float fuel = 50;

// ––––– GENERAL FUNCTIONS ––––– //

void draw_text(ShaderProgram* shader_program, GLuint font_texture_id, std::string text, float font_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs�one for
    // each character. Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++)
    {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their
        //    position relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (font_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }
    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    shader_program->set_model_matrix(model_matrix);
    glUseProgram(shader_program->get_program_id());

    glVertexAttribPointer(shader_program->get_position_attribute(), 2, GL_FLOAT, false, 0,
        vertices.data());
    glEnableVertexAttribArray(shader_program->get_position_attribute());

    glVertexAttribPointer(shader_program->get_tex_coordinate_attribute(), 2, GL_FLOAT,
        false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(shader_program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(shader_program->get_position_attribute());
    glDisableVertexAttribArray(shader_program->get_tex_coordinate_attribute());
}

GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Samus lands the Gunship!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // ––––– VIDEO ––––– //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_program.set_projection_matrix(g_projection_matrix);
    g_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // ––––– BGM ––––– //
    Mix_OpenAudio(CD_QUAL_FREQ, MIX_DEFAULT_FORMAT, AUDIO_CHAN_AMT, AUDIO_BUFF_SIZE);

    // STEP 1: Have openGL generate a pointer to your music file
    g_music = Mix_LoadMUS(BGM_FILEPATH); // works only with mp3 files

    // STEP 2: Play music
    Mix_PlayMusic(
                  g_music,  // music file
                  -1        // -1 means loop forever; 0 means play once, look never
                  );

    // STEP 3: Set initial volume
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2.0);

    // ––––– SFX ––––– //
    g_boost_sfx = Mix_LoadWAV(SFX_FILEPATH);

    // ––––– PLATFORMS ––––– //
    GLuint platform_texture_id = load_texture(PLATFORM_FILEPATH);
    GLuint mountain_texture_id = load_texture(MOUNTAIN_FILEPATH);
    g_font_texture_id = load_texture(FONTSHEET_FILEPATH);

    g_state.platforms = new Entity[PLATFORM_COUNT];
    g_state.mountain = new Entity[MOUNTAIN_COUNT];

    // Set the type of every platform entity to PLATFORM
    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        g_state.platforms[i].set_texture_id(platform_texture_id);
        g_state.platforms[i].set_position(glm::vec3(i - PLATFORM_COUNT / 2.0f, -3.2f, 0.0f));
        g_state.platforms[i].set_width(1.0f);
        g_state.platforms[i].set_height(1.0f);
        g_state.platforms[i].set_entity_type(PLATFORM);
        g_state.platforms[i].update(0.0f, NULL, NULL, 0);
    }

    int count = 0;
    for (int j = 0; j < 3; j++)
    {
        for (int i = 0; i < 2; i++)
        {
            g_state.mountain[count].set_texture_id(mountain_texture_id);
            g_state.mountain[count].set_position(glm::vec3(3.0f + i, -2.2f + (j), 0.0f));
            g_state.mountain[count].set_width(1.0f);
            g_state.mountain[count].set_height(1.0f);
            g_state.mountain[count].set_entity_type(PLATFORM);
            g_state.mountain[count].update(0.0f, NULL, NULL, 0);
            g_state.mountain[count].set_mountain(true);
            count++;
        }
    }
    g_state.mountain[4].set_land_spot(true);
    g_state.mountain[5].set_land_spot(true);


    // ––––– PLAYER (Rocket) ––––– //
    GLuint player_texture_id = load_texture(SPRITESHEET_FILEPATH);

    int player_walking_animation[4][4] =
    {
        { 1, 5, 9, 13 },  // for Rocket to move to the left,
        { 3, 7, 11, 15 }, // for Rocket to move to the right,
        { 2, 6, 10, 14 }, // for Rocket to move upwards,
        { 0, 4, 8, 12 }   // for Rocket to move downwards
    };

    g_state.player = new Entity(
        player_texture_id,           // texture id
        0.0f,                        // speed
        glm::vec3(0.0f, -0.1f, 0.0f), // acceleration
        0.005f,                        // boosting power
        player_walking_animation,    // animation index sets
        0.0f,                        // animation time
        4,                           // animation frame amount
        0,                           // current animation index
        4,                           // animation column amount
        4,                           // animation row amount
        0.8f,                        // width
        1.0f,                        // height
        PLAYER
    );
    g_state.player->set_position(glm::vec3(-2, -2.2f, 0));

    // ––––– GENERAL ––––– //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_game_is_running = false;
                        break;

                    //case SDLK_SPACE:
                    //    // boost
                    //    //if (g_state.player->get_collided_bottom())        ADD FUEL LOGIC LATER
                    //    {
                    //        g_state.player->boost();
                    //        Mix_PlayChannel(NEXT_CHNL, g_boost_sfx, 0);
                    //    }
                    //    break;

                    case SDLK_h:
                        // Stop music
                        Mix_HaltMusic();
                        break;

                    case SDLK_p:
                        Mix_PlayMusic(g_music, -1);

                    default:
                        break;
                }

            default:
                break;
        }
    }

    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    if (!gameover)
    {
        if (key_state[SDL_SCANCODE_SPACE] && fuel > 0)
        {
            g_state.player->boost();
            if (!key_state[SDL_SCANCODE_LEFT] && !key_state[SDL_SCANCODE_RIGHT])
            {
                g_state.player->face_up();
                Mix_PlayChannel(NEXT_CHNL, g_boost_sfx, 0);
                fuel -= 0.001;
            }
        }
        if (key_state[SDL_SCANCODE_LEFT] && fuel > 0)
        {
            g_state.player->move_left();
            //My sprite sheet is a little wrong
            g_state.player->face_right();
            Mix_PlayChannel(NEXT_CHNL, g_boost_sfx, 0);
            fuel -= 0.001;

        }
        else if (key_state[SDL_SCANCODE_RIGHT] && fuel > 0)
        {
            g_state.player->move_right();
            //My sprite sheet is a little wrong
            g_state.player->face_left();
            Mix_PlayChannel(NEXT_CHNL, g_boost_sfx, 0);
            fuel -= 0.001;
        }
        else
        {
            g_state.player->set_acceleration(glm::vec3(0, g_state.player->get_acceleration().y, 0));
        }
        if (fuel <= 0)
        {
            fuel = 0;
        }

        if (glm::length(g_state.player->get_movement()) > 1.0f)
        {
            g_state.player->normalise_movement();
        }
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
        g_state.player->update(FIXED_TIMESTEP, NULL, g_state.platforms, PLATFORM_COUNT);
        g_state.player->update(FIXED_TIMESTEP, NULL, g_state.mountain, MOUNTAIN_COUNT);
        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;

    if (g_state.player->get_losecon())
    {
        gameover = true;
        lost = true;
    }
    if (g_state.player->get_wincon())
    {
        gameover = true;
        won = true;
    }

    if(g_state.player->get_position().x >= 5 || g_state.player->get_position().x <= -5 || g_state.player->get_position().y >= 4.5f || g_state.player->get_position().y <= -3.75f)
    {
        gameover = true;
        lost = true;
    }
    
    if (gameover)
    {
        g_state.player->set_acceleration(glm::vec3(0, 0, 0));
        g_state.player->set_velocity(glm::vec3(0, 0, 0));
    }
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    g_state.player->render(&g_program);

    for (int i = 0; i < PLATFORM_COUNT; i++) g_state.platforms[i].render(&g_program);
    for (int i = 0; i < MOUNTAIN_COUNT; i++) g_state.mountain[i].render(&g_program);

    if (gameover)
    {
        if(won)
        {
            draw_text(&g_program, g_font_texture_id, "You did it!", 0.5f, 0.0005f,
                glm::vec3(-2.5f, 2.0f, 0.0f));
        }
        else
        {
            draw_text(&g_program, g_font_texture_id, "You failed everyone!", 0.4f, 0.0005f,
                glm::vec3(-3.8f, 2.0f, 0.0f));
        }
    }

    draw_text(&g_program, g_font_texture_id, "fuel is " + std::to_string(fuel), 0.6f, 0.0005f,
        glm::vec3(-4.7f, 2.7f, 0.0f));

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

    delete [] g_state.platforms;
    delete g_state.player;
}

// ––––– GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
