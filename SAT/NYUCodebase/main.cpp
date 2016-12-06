#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include <vector>
#include <string>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

class Vector3{
public:
    Vector3();
    Vector3(float xs, float ys, float zs);
    float x;
    float y;
    float z;
};

class Entity{
public:
    Matrix matrix;
    Vector3 position;
    Vector3 velocity;
    Vector3 direction;
    float rotation;
};

SDL_Window* displayWindow;
ShaderProgram *program;
Matrix projectionMatrix; // aspect ratio
Matrix modelMatrix;
Matrix modelMatrix2;
Matrix modelMatrix3;
Matrix viewMatrix;
SDL_Event event;
float lastFrameTicks= 0.0f;
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
bool done = false;
Entity juan; //
Entity too;
Entity three;
std::vector<Vector3> e1Points;
std::vector<Vector3> e2Points;
std::vector<Vector3> e3Points;
int count=0;


Vector3::Vector3(){}

Vector3::Vector3(float xs, float ys, float zs){
    x=xs;
    y=ys;
    z=zs;
}

bool testSATSeparationForEdge(float edgeX, float edgeY, const std::vector<Vector3> &points1, const std::vector<Vector3> &points2) {
    float normalX = -edgeY;
    float normalY = edgeX;
    float len = sqrtf(normalX*normalX + normalY*normalY);
    normalX /= len;
    normalY /= len;
    
    std::vector<float> e1Projected;
    std::vector<float> e2Projected;
    
    for(int i=0; i < points1.size(); i++) {
        e1Projected.push_back(points1[i].x * normalX + points1[i].y * normalY);
    }
    for(int i=0; i < points2.size(); i++) {
        e2Projected.push_back(points2[i].x * normalX + points2[i].y * normalY);
    }
    
    std::sort(e1Projected.begin(), e1Projected.end());
    std::sort(e2Projected.begin(), e2Projected.end());
    
    float e1Min = e1Projected[0];
    float e1Max = e1Projected[e1Projected.size()-1];
    float e2Min = e2Projected[0];
    float e2Max = e2Projected[e2Projected.size()-1];
    float e1Width = fabs(e1Max-e1Min);
    float e2Width = fabs(e2Max-e2Min);
    float e1Center = e1Min + (e1Width/2.0);
    float e2Center = e2Min + (e2Width/2.0);
    float dist = fabs(e1Center-e2Center);
    float p = dist - ((e1Width+e2Width)/2.0);
    
    if(p < 0) {
        return true;
    }
    return false;
}

bool checkSATCollision(const std::vector<Vector3> &e1Points, const std::vector<Vector3> &e2Points) {
    for(int i=0; i < e1Points.size(); i++) {
        float edgeX, edgeY;
        
        if(i == e1Points.size()-1) {
            edgeX = e1Points[0].x - e1Points[i].x;
            edgeY = e1Points[0].y - e1Points[i].y;
        } else {
            edgeX = e1Points[i+1].x - e1Points[i].x;
            edgeY = e1Points[i+1].y - e1Points[i].y;
        }
        
        bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
        if(!result) {
            return false;
        }
    }
    for(int i=0; i < e2Points.size(); i++) {
        float edgeX, edgeY;
        
        if(i == e2Points.size()-1) {
            edgeX = e2Points[0].x - e2Points[i].x;
            edgeY = e2Points[0].y - e2Points[i].y;
        } else {
            edgeX = e2Points[i+1].x - e2Points[i].x;
            edgeY = e2Points[i+1].y - e2Points[i].y;
        }
        bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points);
        if(!result) {
            return false;
        }
    }
    return true;
}


void Setup(){
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    displayWindow = SDL_CreateWindow("SAT", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(0, 0, 640, 360);
    program = new ShaderProgram(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    projectionMatrix.setOrthoProjection(-6.0f, 6.0f, -3.5f, 3.5f, -1.0f, 1.0f);
    glUseProgram(program->programID);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    juan.position.x=-4;
    juan.position.y=-2;
    juan.rotation=170;
    juan.velocity.x=4;
    juan.velocity.y=4;
    juan.direction.x=sin(juan.rotation);
    juan.direction.y=cos(juan.rotation);
    too.position.x=4;
    too.position.y=1;
    too.rotation=130;
    too.velocity.x=4;
    too.velocity.y=4;
    too.direction.x=sin(too.rotation);
    too.direction.y=cos(too.rotation);
    three.position.x=0;
    three.position.y=0;
    three.rotation=1;
    three.velocity.x=4;
    three.velocity.y=4;
    e1Points.push_back(Vector3(0,0,0));
    e1Points.push_back(Vector3(0,0,0));
    e1Points.push_back(Vector3(0,0,0));
    e1Points.push_back(Vector3(0,0,0));
    
    
    e2Points.push_back(Vector3(0,0,0));
    e2Points.push_back(Vector3(0,0,0));
    e2Points.push_back(Vector3(0,0,0));
    e2Points.push_back(Vector3(0,0,0));
    
    e3Points.push_back(Vector3(0,0,0));
    e3Points.push_back(Vector3(0,0,0));
    e3Points.push_back(Vector3(0,0,0));
    e3Points.push_back(Vector3(0,0,0));

}


void ProcessEvents(){
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
    }
}

void collision(Entity *e){
    //top wall
    if((e->position.y)>= 3.5){
         e->direction.y= -fabs(e->direction.y);
        e->velocity.x=4;
        e->velocity.y=4;
    }
    //bottom wall
    if((e->position.y)<= -3.5){
        e->direction.y= fabs( e->direction.y);
        e->velocity.x=4;
        e->velocity.y=4;
    }
    
    //left wall
    if((e->position.x)<= -6.0){
         e->direction.x = fabs(e->direction.y);
        e->velocity.x=4;
        e->velocity.y=4;
    }
    
    //right wall
    if((e->position.x)>= 6.0){
        e->direction.x = -fabs(e->direction.y);
        e->velocity.x=4;
        e->velocity.y=4;
    }
    
}

void fixUpdate(float elapsed){
    three.rotation += elapsed*FIXED_TIMESTEP;
    juan.position.x += juan.direction.x*elapsed*juan.velocity.x;
    juan.position.y += juan.direction.y*elapsed*juan.velocity.y;
    too.position.x += too.direction.x*elapsed*too.velocity.x;
    too.position.y += too.direction.y*elapsed*too.velocity.y;
    collision(&juan);
    collision(&too);
    e1Points[0]=Vector3(-0.8*cos(juan.rotation*(3.1415926/180))+(-0.8*(-sin(juan.rotation*(3.1415926/180))))+juan.position.x, -0.8*sin(juan.rotation*(3.1415926/180))-0.8*(cos(juan.rotation*(3.1415926/180)))+juan.position.y, 1);
    e1Points[1]= Vector3(0.8*cos(juan.rotation*(3.1415926/180))-0.8*(-sin(juan.rotation*(3.1415926/180)))+juan.position.x, 0.8*sin(juan.rotation*(3.1415926/180))-0.8*(cos(juan.rotation*(3.1415926/180)))+juan.position.y, 1);
    e1Points[2] =Vector3(0.8*cos(juan.rotation*(3.1415926/180))+0.8*(-sin(juan.rotation*(3.1415926/180)))+juan.position.x, 0.8*sin(juan.rotation*(3.1415926/180))+0.8*(cos(juan.rotation*(3.1415926/180)))+juan.position.y, 1);
    e1Points[3]=Vector3(-0.8*cos(juan.rotation*(3.1415926/180))+0.8*(-sin(juan.rotation*(3.1415926/180)))+juan.position.x, -0.8*sin(juan.rotation*(3.1415926/180))+0.8*(cos(juan.rotation*(3.1415926/180)))+juan.position.y, 1);
    
    e2Points[0]=Vector3(-0.8*cos(three.rotation*(3.1415926/180))-0.8*(-sin(three.rotation*(3.1415926/180)))+three.position.x, -0.8*sin(three.rotation*(3.1415926/180))-0.8*(cos(three.rotation*(3.1415926/180)))+three.position.y, 1);
    e2Points[1]= Vector3(0.8*cos(three.rotation*(3.1415926/180))-0.8*(-sin(three.rotation*(3.1415926/180)))+three.position.x, 0.8*sin(three.rotation*(3.1415926/180))-0.8*(cos(three.rotation*(3.1415926/180)))+three.position.y, 1);
    e2Points[2] =Vector3(0.8*cos(three.rotation*(3.1415926/180))+0.8*(-sin(three.rotation*(3.1415926/180)))+three.position.x, 0.8*sin(three.rotation*(3.1415926/180))+0.8*(cos(three.rotation*(3.1415926/180)))+three.position.y, 1);
    e2Points[3]=Vector3(-0.8*cos(three.rotation*(3.1415926/180))+0.8*(-sin(three.rotation*(3.1415926/180)))+three.position.x, -0.8*sin(three.rotation*(3.1415926/180))+0.8*(cos(three.rotation*(3.1415926/180)))+three.position.y, 1);
    
    e3Points[0]=Vector3(-0.8*1.4*cos(too.rotation*(3.1415926/180))-0.8*1.4*(-sin(too.rotation*(3.1415926/180)))+too.position.x, -0.8*sin(too.rotation*(3.1415926/180))-0.8*(cos(too.rotation*(3.1415926/180)))+too.position.y, 1);
    e3Points[1]=Vector3(0.8*1.4*cos(too.rotation*(3.1415926/180))-0.8*1.4*(-sin(too.rotation*(3.1415926/180)))+too.position.x, 0.8*sin(too.rotation*(3.1415926/180))-0.8*(cos(too.rotation*(3.1415926/180)))+too.position.y, 1);
    e3Points[2]=Vector3(0.8*1.4*cos(too.rotation*(3.1415926/180))+0.8*1.4*(-sin(too.rotation*(3.1415926/180)))+too.position.x, 0.8*sin(too.rotation*(3.1415926/180))+0.8*(cos(too.rotation*(3.1415926/180)))+too.position.y, 1);
    e3Points[3]=Vector3(-0.8*1.4*cos(too.rotation*(3.1415926/180))+0.8*1.4*(-sin(too.rotation*(3.1415926/180)))+too.position.x, -0.8*sin(too.rotation*(3.1415926/180))+0.8*(cos(too.rotation*(3.1415926/180)))+too.position.y, 1);
    //collision
    std::cout<<juan.velocity.x<<std::endl;
    if (checkSATCollision(e1Points,e2Points)== true){
        if (juan.direction.x>0){
        juan.direction.x = -juan.direction.x;
        }
        else{
            juan.direction.x=fabs(juan.direction.x);
        }
        if (juan.direction.y>0){
        juan.direction.y = -juan.direction.y;
        }
        else{
            juan.direction.y=fabs(juan.direction.y);
        }
    }
    if (checkSATCollision(e2Points,e3Points)== true){
        if (too.direction.x>0){
            too.direction.x = -too.direction.x;
        }
        else{
            too.direction.x=fabs(too.direction.x);
        }
        if (too.direction.y>0){
            too.direction.y = -too.direction.y;
        }
        else{
            too.direction.y=fabs(too.direction.y);
        }
        three.rotation=1;
    }
    if (checkSATCollision(e1Points,e3Points)== true){
        
        if(checkSATCollision(e1Points,e2Points)== true || checkSATCollision(e2Points, e3Points) == true){
            juan.position.x=-4;
            juan.position.y=-2;
            too.position.x=4;
            too.position.y=1;
        }
        else{
        if (juan.direction.x>0){
            juan.direction.x = -juan.direction.x;
        }
        else{
            juan.direction.x=fabs(juan.direction.x);
        }
        if (juan.direction.y>0){
            juan.direction.y = -juan.direction.y;
        }
        else{
            juan.direction.y=fabs(juan.direction.y);
        }
        
        if (too.direction.x>0){
            too.direction.x = -too.direction.x;
        }
        else{
            too.direction.x=fabs(too.direction.x);
        }
        if (too.direction.y>0){
            too.direction.y = -too.direction.y;
        }
        else{
            too.direction.y=fabs(too.direction.y);
        }
        }
    }
    
    
}


void Update(){
    float ticks = (float)SDL_GetTicks()/1000.0f;
    float elapsed = ticks- lastFrameTicks;
    lastFrameTicks=ticks;
    float fixedElapsed = elapsed;
    if(fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
        fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
    }
    while (fixedElapsed >= FIXED_TIMESTEP ) {
        fixedElapsed -= FIXED_TIMESTEP;
        fixUpdate(FIXED_TIMESTEP);
    }
    fixUpdate(fixedElapsed);
}

void Render(){
    glClear(GL_COLOR_BUFFER_BIT);
    modelMatrix.identity();
    modelMatrix.Translate(juan.position.x, juan.position.y, 0);
    modelMatrix.Rotate(juan.rotation*(3.1415926/180));
    program->setModelMatrix(modelMatrix);
    program->setProjectionMatrix(projectionMatrix);
    program->setViewMatrix(viewMatrix);
    float vertices[] = {-0.8, -0.8, 0.8, -0.8, 0.8, 0.8, -0.8, -0.8, 0.8, 0.8, -0.8, 0.8};
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program->positionAttribute);
    modelMatrix2.identity();
    modelMatrix2.Translate(too.position.x, too.position.y, 0);
    modelMatrix2.Scale(1.4f,1.0f, 1.0f);
    modelMatrix2.Rotate(too.rotation*(3.1415926/180));
    program->setModelMatrix(modelMatrix2);
    float vertices2[] = {-0.8, -0.8, 0.8, -0.8, 0.8, 0.8, -0.8, -0.8, 0.8, 0.8, -0.8, 0.8};
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
    glEnableVertexAttribArray(program->positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    modelMatrix3.Translate(three.position.x, three.position.y, 0);
    modelMatrix3.Rotate(three.rotation*(3.1415926/180));
    program->setModelMatrix(modelMatrix3);
    float vertices3[] = {-0.8, -0.8, 0.8, -0.8, 0.8, 0.8, -0.8, -0.8, 0.8, 0.8, -0.8, 0.8};
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices3);
    glEnableVertexAttribArray(program->positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    SDL_GL_SwapWindow(displayWindow);
    
}

int main(int argc, char *argv[])
{
    Setup();
    
    while (!done) {
        ProcessEvents();
        //LOOP
        Update();
        Render();
        
    }
    SDL_Quit();
    return 0;
}





