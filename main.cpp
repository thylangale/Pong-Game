#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;
bool gameStarted = false;

ShaderProgram program;
glm::mat4 viewMatrix, player1Matrix, player2Matrix, ballMatrix, ballMatrix2, projectionMatrix;

glm::vec3 player1_position = glm::vec3(4.75f, 0, 0);
glm::vec3 player1_movement = glm::vec3(0, 0, 0);
float player_speed = 2.0f;

glm::vec3 player2_position = glm::vec3(-4.75f, 0, 0);
glm::vec3 player2_movement = glm::vec3(0, 0, 0);

glm::vec3 ball_position = glm::vec3(0, 0, 0);
glm::vec3 ball_movement = glm::vec3(0, 0, 0);
float ball_speed = 4.0f;

GLuint player1TextureID;
GLuint player2TextureID;
GLuint ballTextureID;

GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);

    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);
    return textureID;
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Thy-Lan Gale - Project 2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, 640, 480);

    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    viewMatrix = glm::mat4(1.0f);
    player1Matrix = glm::mat4(1.0f);
    player2Matrix = glm::mat4(1.0f);
    ballMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);


    glUseProgram(program.programID);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //background color

    glEnable(GL_BLEND);
    // Good setting for transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    player1TextureID = LoadTexture("player1_paddle.png");
    player2TextureID = LoadTexture("player2_paddle.png");
    ballTextureID = LoadTexture("ball.png");
}

void ProcessInput() { //looks for events

    player1_movement = glm::vec3(0);
    player2_movement = glm::vec3(0);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        //Move the player left
                        break;
                    case SDLK_RIGHT:
                        //Move the player right
                        break;
                    case SDLK_SPACE:
                        //Some sort of action
                        break;
                }
                break;  //SDL_KEYDOWN
        }
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_UP]) {
        if (player1_position.y < 3.0f) {
            player1_movement.y = 1.0f;
        }
        gameStarted = true;
    }
    else if (keys[SDL_SCANCODE_DOWN]) {
        if (player1_position.y > -3.0f) {
            player1_movement.y = -1.0f;
        }
        gameStarted = true;
    }

    if (keys[SDL_SCANCODE_S]) {
        if (player2_position.y > -3.0f) {
            player2_movement.y = -1.0f;
        }
        gameStarted = true;
    }

    else if (keys[SDL_SCANCODE_W]) {
        if (player2_position.y < 3.0f) {
            player2_movement.y = 1.0f;
        }
        gameStarted = true;
    }

    if (glm::length(player1_movement) > 1.0f) {
        player1_movement = glm::normalize(player1_movement);
    }
    
}

float lastTicks = 0.0f;

void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    player1_position += player1_movement * player_speed * deltaTime;

    player1Matrix = glm::mat4(1.0f);
    player1Matrix = glm::translate(player1Matrix, player1_position);
    player1Matrix = glm::scale(player1Matrix, glm::vec3(2.0f, 2.0f, 1.0f));

    player2_position += player2_movement * player_speed * deltaTime;

    player2Matrix = glm::mat4(1.0f);
    player2Matrix = glm::translate(player2Matrix, player2_position);
    player2Matrix = glm::scale(player2Matrix, glm::vec3(2.0f, 2.0f, 1.0f));
    
    //collision detectment and hitting walls
    if (gameStarted) {
        if (ball_movement.x == 0 && ball_movement.y == 0) {
            ball_movement.x = 0.5f;
            ball_movement.y = 0.5f;
        }

        if (ball_position.y > 3.5f) { //if the ball hits the top wall
            ball_movement.y *= -1.0f;
        }
        if (ball_position.y < -3.5f) { //if the ball hits the bottom wall
            ball_movement.y *= -1.0f;
        }

        if (ball_position.x > 5.0f || ball_position.x < -5.0f) { //if the ball leaves the game area
            gameIsRunning = false;
        }

        if ((ball_position.x > 4.5f && ball_position.x < 5.0f) && (ball_position.y > (player1_position.y - 1.0f) && ball_position.y < (player1_position.y + 1.0f))) {//if the ball hits player 1 paddle
            ball_movement.x *= -1.0f;
        }
        if ((ball_position.x < -4.5f && ball_position.x > -5.0f) && (ball_position.y > (player2_position.y - 1.0f) && ball_position.y < (player2_position.y + 1.0f))) {//if the ball hits player 2 paddle
            ball_movement.x *= -1.0f;
        }        

        if (glm::length(ball_movement) > 1.0f) {
            ball_movement = glm::normalize(ball_movement);
        }
        ball_position += ball_movement * ball_speed * deltaTime;

        ballMatrix = glm::mat4(1.0f);
        ballMatrix = glm::translate(ballMatrix, ball_position);
    }
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);

    program.SetModelMatrix(player1Matrix);
    glBindTexture(GL_TEXTURE_2D, player1TextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    program.SetModelMatrix(player2Matrix);
    glBindTexture(GL_TEXTURE_2D, player2TextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    program.SetModelMatrix(ballMatrix);
    glBindTexture(GL_TEXTURE_2D, ballTextureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);

    SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();

    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }

    Shutdown();
    return 0;
}