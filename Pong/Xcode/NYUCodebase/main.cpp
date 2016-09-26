#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
/**************************************************
 PONG
 instructions:
 press space to pause
 press left click to start game
 
 *************************************************/


SDL_Window* displayWindow;
ShaderProgram *program;
Matrix projectionMatrix; // aspect ratio
Matrix modelMatrixA;
Matrix modelMatrixB;
Matrix ball;
Matrix viewMatrix;
float lastFrameTicks= 0.0f;
float playerWidth = 0.5f;
float playerHeight = 1.5f;
float ballside = 0.2f;
float angle= angle = -130* 3.1415/180;
float playerAX = -6.0f+0.5;
float playerAY = 0.0;
int Ascore = 0;
float playerBX = 6-0.5;
float playerBY = playerAY;
int Bscore= 0;
float ballX=(playerAX+playerBX)/2;
float ballY=(playerAY+playerBY)/2;
float speed = 0;
float yDir = sin(angle);
float xDir = cos(angle);
SDL_Event event;
bool done = false;

void Setup(){
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    displayWindow = SDL_CreateWindow("PONG", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    #ifdef _WINDOWS
        glewInit();
    #endif
    glViewport(0, 0, 640, 360); // how big window is
    program = new ShaderProgram(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    projectionMatrix.setOrthoProjection(-6.0f, 6.0f, -3.5f, 3.5f, -1.0f, 1.0f);
    glUseProgram(program->programID);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
}

void ProcessEvents(){
    while (SDL_PollEvent(&event)) {
        
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
        else if(event.type ==SDL_KEYDOWN){
            if(event.key.keysym.scancode ==SDL_SCANCODE_SPACE){
                //do an action if spacebar is pressed
                if (speed == 4){
                    speed = 0;
                }
            }
        }
        else if(event.type == SDL_MOUSEBUTTONDOWN){
            speed = 4;
        }

    }
}

void Update(){
    
    float ticks = (float)SDL_GetTicks()/1000.0f;
    float elapsed = ticks- lastFrameTicks;
    lastFrameTicks=ticks;
    
    glClear(GL_COLOR_BUFFER_BIT);
    //LOOP
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if(keys[SDL_SCANCODE_W]){
        if((playerAY+playerHeight/2)<=3.5)
            playerAY += elapsed *2.0;
    }
    if(keys[SDL_SCANCODE_S]){
        if((playerAY- playerHeight/2) >=-3.5)
            playerAY -= elapsed *2.0;
    }
    if(keys[SDL_SCANCODE_UP]){
        if((playerBY+playerHeight/2)<=3.5)
            playerBY += elapsed *2.0;
    }
    if(keys[SDL_SCANCODE_DOWN]){
        if((playerBY- playerHeight/2) >=-3.5)
            playerBY -= elapsed *2.0;
    }
    modelMatrixA.identity();
    modelMatrixA.Translate(playerAX, playerAY, 0);
    program->setModelMatrix(modelMatrixA);
    program->setProjectionMatrix(projectionMatrix);
    program->setViewMatrix(viewMatrix);
    //player 1
    float vertices[] = {-0.25f, -0.75f, 0.25f, -0.75f, 0.25f, 0.75f, -0.25f, -0.75f, 0.25f, 0.75f, -0.25f, 0.75f};
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program->positionAttribute);
    
    
    //player2
    modelMatrixB.identity();
    modelMatrixB.Translate(playerBX, playerBY, 0);
    program->setModelMatrix(modelMatrixB);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program->positionAttribute);
    
    //ball
    ball.identity();
    ballX += xDir*elapsed*speed;
    ballY += yDir*elapsed*speed;
    ball.Translate(ballX, ballY,0);
    float ballLeft =ballX-(ballside/2);
    float ballTop = ballY-(ballside/2);
    float ballBot = ballY+(ballside/2);
    
    float aRight =playerAX+(playerWidth/2);
    float aTop = playerAY-(playerHeight/2);
    float aBot = playerAY+(playerHeight/2);
    
    float bLeft = playerBX-(playerWidth/2);
    float bTop = playerBY-(playerHeight/2);
    float bBot = playerBY+(playerHeight/2);
    
    
    //left block
    if(ballLeft < aRight && ballBot > aTop && ballTop < aBot){
        //printf("true");
        xDir=fabs(xDir);
        yDir =ballY-playerAY;
    }
    
    //right block
    if(ballLeft > bLeft && ballBot > bTop && ballTop < bBot){
        //printf("true");
        xDir=-fabs(xDir);
        yDir =ballY-playerBY;
    }
    
    //top wall
    if((ballY)>= 3.5){
        yDir= -fabs(yDir);
    }
    //bottom wall
    if((ballY)<= -3.5){
        yDir = fabs(yDir);
    }
    
    //left wall
    if((ballX)<= -6.0){
        ballX =0-0.2;
        ballY =0-0.2;
        speed =0;
        Bscore+=1;
    }
    
    //right wall
    if((ballX)>= 6.0){
        ballX =0-0.2;
        ballY =0-0.2;
        speed =0;
        Ascore+=1;
    }
    
    program->setModelMatrix(ball);
    float verticesball[] = {-0.1, -0.1, 0.1, -0.1, 0.1, 0.1, -0.1, -0.1, 0.1, 0.1, -0.1, 0.1};
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, verticesball);
    glEnableVertexAttribArray(program->positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program->positionAttribute);
    
    SDL_GL_SwapWindow(displayWindow);
}

int main(int argc, char *argv[])
{
    Setup();
    while (!done) {
        ProcessEvents();
        Update();
    }
    
    SDL_Quit();
    return 0;
}
