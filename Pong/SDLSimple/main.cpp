/**
* Author: Alok Aenugu
* Assignment: Pong
* Date due: 2024-10-12, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>  

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include <vector>
#include "cmath"
#include <ctime>

enum AppStatus { RUNNING, TERMINATED };

constexpr float WINDOW_SIZE_MULT = 1.0f;

constexpr int WINDOW_WIDTH = 640 * WINDOW_SIZE_MULT,
WINDOW_HEIGHT = 480 * WINDOW_SIZE_MULT;

constexpr float BG_RED = 0.9765625f,
BG_GREEN = 0.97265625f,
BG_BLUE = 0.9609375f,
BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr GLint NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL = 0;
constexpr GLint TEXTURE_BORDER = 0;

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

void draw_text(ShaderProgram* shader_program, GLuint font_texture_id, std::string text,
    float font_size, float spacing, glm::vec3 position);

constexpr char BALL_SPRITE_FILEPATH[] = "SmashBall.png",
PADDLE1_SPRITE_FILEPATH[] = "Captain1.png",
PADDLE2_SPRITE_FILEPATH[] = "Captain2.png";
constexpr char FONTSHEET_FILEPATH[] = "font1.png";
constexpr int FONTBANK_SIZE = 16;

constexpr float MINIMUM_COLLISION_DISTANCE = 1.0f;
constexpr glm::vec3 INIT_SCALE_PADDLE1 = glm::vec3(1.0f, 1.5f, 0.0f),
INIT_POS_PADDLE1 = glm::vec3(-4.0f, 0.0f, 0.0f),
INIT_SCALE_PADDLE2 = glm::vec3(1.0f, 1.5f, 0.0f),
INIT_POS_PADDLE2 = glm::vec3(4.0f, 0.0f, 0.0f),
INIT_SCALE_BALL = glm::vec3(1.0f, 1.0f, 1.0f),
INIT_POS_BALL = glm::vec3(0.0f, 0.0f, 0.0f);

SDL_Window* g_display_window;

AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();
glm::mat4 g_view_matrix, g_BALL_matrix, g_projection_matrix, g_PADDLE1_matrix, g_PADDLE2_matrix;

float g_previous_ticks = 0.0f;


//TEXTURES
GLuint g_font_texture_id;
GLuint g_BALL_texture_id;
GLuint g_PADDLE1_texture_id;
GLuint g_PADDLE2_texture_id;

//Ball Initial and Change Vectors
glm::vec3 g_BALL_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 BALL_SPEED = glm::vec3(0.0f, 0.0f, 0.0f);
bool started = false;
bool passed = false;
bool gameover = false;
bool computer_player = false;
bool P1WIN = false;
bool P2WIN = false;

//PADDLE 1 Initial and Change Vectors
glm::vec3 PADDLE1_SPEED = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_PADDLE1_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_PADDLE1_scale = glm::vec3(0.0f, 0.0f, 0.0f);  // scale trigger vector
glm::vec3 g_PADDLE1_size = glm::vec3(1.0f, 1.0f, 0.0f);  // scale accumulator vector

//POSSIBLY DELETE SCALE AND SIZE LATERRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR

//PADDLE 2 Initial and Change Vectors
glm::vec3 PADDLE2_SPEED = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_PADDLE2_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_PADDLE2_scale = glm::vec3(0.0f, 0.0f, 0.0f);  // scale trigger vector
glm::vec3 g_PADDLE2_size = glm::vec3(1.0f, 1.0f, 0.0f);  // scale accumulator vector

void initialise();
void process_input();
void update();
void render();
void shutdown();

//FONTS
void draw_text(ShaderProgram* shader_program, GLuint font_texture_id, std::string text, float font_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for
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
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{

    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Pong",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);


    if (g_display_window == nullptr) shutdown();

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_BALL_matrix = glm::mat4(1.0f);
    g_PADDLE1_matrix = glm::mat4(1.0f);
    g_PADDLE2_matrix = glm::mat4(1.0f);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    g_BALL_texture_id = load_texture(BALL_SPRITE_FILEPATH);
    g_PADDLE1_texture_id = load_texture(PADDLE1_SPRITE_FILEPATH);
    g_PADDLE2_texture_id = load_texture(PADDLE2_SPRITE_FILEPATH);
    g_font_texture_id = load_texture(FONTSHEET_FILEPATH);

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_app_status = TERMINATED;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_q: g_app_status = TERMINATED; break;
            default: break;
            }
        default:
            break;
        }
    }
    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    //PADDLE INPUTS TO MOVEMENTS
    float paddle1_top = g_PADDLE1_position.y + (INIT_SCALE_PADDLE1.y / 2.0f);
    float paddle1_bot = g_PADDLE1_position.y - (INIT_SCALE_PADDLE1.y / 2.0f);
    float paddle2_top = g_PADDLE2_position.y + (INIT_SCALE_PADDLE2.y / 2.0f);
    float paddle2_bot = g_PADDLE2_position.y - (INIT_SCALE_PADDLE2.y / 2.0f);
    if (key_state[SDL_SCANCODE_W])
    {
        if(gameover)
        {
            PADDLE1_SPEED = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        else if (paddle1_top < 3.75f)
        {
            PADDLE1_SPEED = glm::vec3(0.0f, 5.0f, 0.0f);
        }
        else
        {
            PADDLE1_SPEED = glm::vec3(0.0f, 0.0f, 0.0f);
        }
    }
    else if (key_state[SDL_SCANCODE_S])
    {
        if (gameover)
        {
            PADDLE1_SPEED = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        else if (paddle1_bot > -3.75f)
        {
            PADDLE1_SPEED = glm::vec3(0.0f, -5.0f, 0.0f);
        }
        else
        {
            PADDLE1_SPEED = glm::vec3(0.0f, 0.0f, 0.0f);
        }
    }
    else
    {
        PADDLE1_SPEED = glm::vec3(0.0f, 0.0f, 0.0f);
    }

    if (key_state[SDL_SCANCODE_UP] && !computer_player)
    {
        if (gameover)
        {
            PADDLE2_SPEED = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        else if (paddle2_top < 3.75f)
        {
            PADDLE2_SPEED = glm::vec3(0.0f, 5.0f, 0.0f);
        }
        else
        {
            PADDLE2_SPEED = glm::vec3(0.0f, 0.0f, 0.0f);
        }
    }
    else if (key_state[SDL_SCANCODE_DOWN] && !computer_player)
    {
        if (gameover)
        {
            PADDLE2_SPEED = glm::vec3(0.0f, 0.0f, 0.0f);
        }
        else if (paddle2_bot > -3.75f)
        {
            PADDLE2_SPEED = glm::vec3(0.0f, -5.0f, 0.0f);
        }
        else
        {
            PADDLE2_SPEED = glm::vec3(0.0f, 0.0f, 0.0f);
        }
    }
    else if(!computer_player)
    {
        PADDLE2_SPEED = glm::vec3(0.0f, 0.0f, 0.0f);
    }

    //RANDOMLY START THE BALL IN EITHER DIRECTION AT A 45 DEGREE ANGLE
    float scalex = rand() / (float)RAND_MAX;
    float random_x = -1.0f + scalex * (1 - (-1));
    float scaley = rand() / (float)RAND_MAX;
    float random_y = -1.0f + scaley * (1 - (-1));
    float startX = -1;
    float startY = -1;
    if (random_x >= 0)
    {
        startX = 1;
    }
    if (random_y >= 0)
    {
        startY = 1;
    }
    //ONLY START THE GAME WHEN B IS PRESSED.  B for BEGINNING!!! 
    if (key_state[SDL_SCANCODE_B] && !started)
    {
        BALL_SPEED = glm::vec3(startX / sqrtf(2), startY / sqrtf(2), 0.0f) * 4.0f;
        started = true;
    }

    //SWITCH FROM P2 TO CPU
    if(key_state[SDL_SCANCODE_T])
    {
        PADDLE2_SPEED = glm::vec3(0.0f, -5.0f, 0.0f);
        computer_player = true;
    }
}

void update()
{
    // --- DELTA TIME CALCULATIONS --- //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    float cpu_top = g_PADDLE2_position.y + (INIT_SCALE_PADDLE2.y / 2.0f);
    float cpu_bot = g_PADDLE2_position.y - (INIT_SCALE_PADDLE2.y / 2.0f);
    if ((cpu_bot <= -3.75f || cpu_top >= 3.75f) && computer_player)
    {
        {
            PADDLE2_SPEED = glm::vec3(0.0f, PADDLE2_SPEED.y * -1, 0.0f);
        }
    }

    // GAME OVER LOGIC
    if (gameover)
    {
        PADDLE2_SPEED *= 0;
        PADDLE1_SPEED *= 0;
        BALL_SPEED *= 0;
    }

    // --- PADDLE TRANSLATION BY INPUT --- //
    g_PADDLE1_position += PADDLE1_SPEED * delta_time;
    g_PADDLE1_size += g_PADDLE1_scale * PADDLE1_SPEED * delta_time;

    g_PADDLE2_position += PADDLE2_SPEED * delta_time;
    g_PADDLE2_size += g_PADDLE1_scale * PADDLE2_SPEED * delta_time;

    // --- BALL TRANSLATION --- //
    g_BALL_position += BALL_SPEED * delta_time;

    // --- INITIAL POSITIONS OF PADDLES AND BALL --- //
    g_BALL_matrix = glm::mat4(1.0f);
    g_BALL_matrix = glm::translate(g_BALL_matrix, INIT_POS_BALL);
    g_BALL_matrix = glm::translate(g_BALL_matrix, g_BALL_position);

    g_PADDLE1_matrix = glm::mat4(1.0f);
    g_PADDLE1_matrix = glm::translate(g_PADDLE1_matrix, INIT_POS_PADDLE1);
    g_PADDLE1_matrix = glm::translate(g_PADDLE1_matrix, g_PADDLE1_position);

    g_PADDLE2_matrix = glm::mat4(1.0f);
    g_PADDLE2_matrix = glm::translate(g_PADDLE2_matrix, INIT_POS_PADDLE2);
    g_PADDLE2_matrix = glm::translate(g_PADDLE2_matrix, g_PADDLE2_position);

    // --- INITIAL SCALING OF PADDLES AND BALL --- //
    g_BALL_matrix = glm::scale(g_BALL_matrix, INIT_SCALE_BALL);
    g_PADDLE1_matrix = glm::scale(g_PADDLE1_matrix, INIT_SCALE_PADDLE1);
    g_PADDLE2_matrix = glm::scale(g_PADDLE2_matrix, INIT_SCALE_PADDLE2);

    // --- COLLISION LOGIC --- //

    float ball_top = g_BALL_position.y + INIT_POS_BALL.y + (INIT_SCALE_BALL.y / 2.0f);
    float ball_bottom = g_BALL_position.y + INIT_POS_BALL.y - (INIT_SCALE_BALL.y / 2.0f);
    float ball_right = g_BALL_position.x + INIT_POS_BALL.x + (INIT_SCALE_BALL.x / 2.0f);
    float ball_left = g_BALL_position.x + INIT_POS_BALL.x - (INIT_SCALE_BALL.x / 2.0f);
    if (ball_top >= 3.75f)
    {
        BALL_SPEED.y *= -1;
    }
    else if (ball_bottom <= -3.75f)
    {
        BALL_SPEED.y *= -1;
    }
    if (ball_left <= -5.0f)
    {
        gameover = true;
    }
    else if (ball_right >= 5.0f)
    {
        P1WIN = true;
        gameover = true;
    }

    float paddle1_left = g_PADDLE1_position.x + INIT_POS_PADDLE1.x - (INIT_SCALE_PADDLE1.x / 2.0f);
    float paddle1_right = g_PADDLE1_position.x + INIT_POS_PADDLE1.x + (INIT_SCALE_PADDLE1.x / 2.0f);
    float paddle1_top = g_PADDLE1_position.y + INIT_POS_PADDLE1.y + (INIT_SCALE_PADDLE1.y / 2.0f);
    float paddle1_bottom = g_PADDLE1_position.y + INIT_POS_PADDLE1.y - (INIT_SCALE_PADDLE1.y / 2.0f);

    float paddle2_left = g_PADDLE2_position.x + INIT_POS_PADDLE2.x - (INIT_SCALE_PADDLE2.x / 2.0f);
    float paddle2_right = g_PADDLE2_position.x + INIT_POS_PADDLE2.x + (INIT_SCALE_PADDLE2.x / 2.0f);
    float paddle2_top = g_PADDLE2_position.y + INIT_POS_PADDLE2.y + (INIT_SCALE_PADDLE2.y / 2.0f);
    float paddle2_bottom = g_PADDLE2_position.y + INIT_POS_PADDLE2.y - (INIT_SCALE_PADDLE2.y / 2.0f);

    if (ball_right >= paddle1_left && ball_left <= paddle1_right && (ball_top < paddle1_bottom || ball_bottom > paddle1_top))
    {
        passed = true;
    }
    if (ball_left <= paddle2_right && ball_right >= paddle2_left && (ball_top < paddle2_bottom || ball_bottom > paddle2_top))
    {
        passed = true;
    }
    if (ball_right >= paddle1_left && ball_left <= paddle1_right && ball_top >= paddle1_bottom && ball_bottom <= paddle1_top && !passed)
    {
        BALL_SPEED.x *= -1;
    }
    if (ball_left <= paddle2_right && ball_right >= paddle2_left && ball_top >= paddle2_bottom && ball_bottom <= paddle2_top && !passed)
    {
        BALL_SPEED.x *= -1;
    }

}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render()
{
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

    // Bind texture
    draw_object(g_PADDLE1_matrix, g_PADDLE1_texture_id);
    draw_object(g_PADDLE2_matrix, g_PADDLE2_texture_id);
    draw_object(g_BALL_matrix, g_BALL_texture_id);

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    if(gameover)
    {
        if (P1WIN)
        {
            draw_text(&g_shader_program, g_font_texture_id, "A WINNER IS YOU, P1!", 0.5f, 0.0005f,
                glm::vec3(-4.7f, 2.0f, 0.0f));
        }
        else
        {
            draw_text(&g_shader_program, g_font_texture_id, "A WINNER IS YOU, P2!", 0.5f, 0.0005f,
                glm::vec3(-4.7f, 2.0f, 0.0f));
        }

    }

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    srand(time(NULL));
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