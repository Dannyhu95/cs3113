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


class SheetSprite {
public:
    SheetSprite();
    SheetSprite(unsigned int textureIDD, float U, float V, float widthh, float heightt, float sizee);
    void Draw(ShaderProgram *program);
    float size;
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
};

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
    void draw();
    Vector3 position;
    Vector3 velocity;
    Vector3 size;
    float rotation;
    SheetSprite sprite;
    bool alive= true;
    float cooldown;
};

SDL_Window* displayWindow;
ShaderProgram *program;
Matrix projectionMatrix; // aspect ratio
Matrix modelMatrix;
Matrix modelMatrix2;
Matrix modelMatrix3;
Matrix modelMatrix4;
Matrix modelMatrix5;
Matrix modelMatrix6;
Matrix viewMatrix;
SDL_Event event;
float lastFrameTicks= 0.0f;
bool done = false;
GLuint spriteSheetTexture;
GLuint fontTexture;
SheetSprite playerSprite;
float playerX;
float playerY;
Entity alien;
float playerWidth;
float enemyWidth;
float bulletwidth;
std::vector<Entity> entities;
float enemySpeed= 0.4;
#define MAX_BULLETS 60
int bulletIndex = 0;
int enemyBulletIndex=0;
Entity bullets[MAX_BULLETS];
Entity enemyBullets[MAX_BULLETS];
enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL };
int state;
int score=0;

SheetSprite::SheetSprite(){}

SheetSprite::SheetSprite(unsigned int textureIDD, float U, float V, float widthh, float heightt, float sizee){
    textureID = textureIDD;
    u = U;
    v = V;
    width = widthh;
    height = heightt;
    size = sizee;
}

void SheetSprite::Draw(ShaderProgram *program) {
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLfloat texCoords[] = {
        u,v+height,
        u+width, v,
        u,v,
        u+width, v,
        u, v+height,
        u+width, v+height
    };
    float aspect = width /height;
    float vertices[] {
        -0.5f * size * aspect, -0.5f*size,
        0.5f * size * aspect, 0.5f*size,
        -0.5f * size * aspect, 0.5f*size,
        0.5f * size * aspect, 0.5f*size,
        -0.5f * size * aspect, -0.5f*size,
        0.5f * size * aspect, -0.5f*size
    };
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices); //how big sprite is
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0,  texCoords);//what to render
    glEnableVertexAttribArray(program->texCoordAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}




Vector3::Vector3(){}

Vector3::Vector3(float xs, float ys, float zs){
    x=xs;
    y=ys;
    z=zs;
}


GLuint LoadTexture(const char *image_path) {
    SDL_Surface *surface = IMG_Load(image_path);
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    SDL_FreeSurface(surface);
    return textureID;
}



void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
    float texture_size = 1.0/16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    for(int i=0; i < text.size(); i++) {
        float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
        float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size+spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        }); }
    glUseProgram(program->programID);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}





void Setup(){
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(0, 0, 640, 360); // how big window is
    program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    projectionMatrix.setOrthoProjection(-3.55, 3.55, -2.0f, 2.0f, -1.0f, 1.0f);
    glUseProgram(program->programID);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    spriteSheetTexture = LoadTexture("sheet.png");
    fontTexture= LoadTexture("font1.png");
    playerSprite = SheetSprite(spriteSheetTexture, 425.0f/1024.0f, 468.0f/1024.0f, 93.0f/1024.0f, 84.0f / 1024.0f, 0.5);
    alien.sprite = SheetSprite(spriteSheetTexture, 423.0f/1024.0f, 728.0f/1024.0f, 93.0f/1024.0f, 84.0f / 1024.0f, 0.5);
    playerWidth =(playerSprite.width/playerSprite.height)*playerSprite.size;
    enemyWidth = (alien.sprite.width/alien.sprite.height)*alien.sprite.size;
    playerX = 0.0f;
    playerY = -2.0f + enemyWidth/2;
    float xpos = 1;
    float ypos = 2.0f - enemyWidth/2;
    alien.position.x=-3.5;
    alien.position.y= 2.0f - enemyWidth/2;
    for(int i=0; i < 10; i++) {
        alien.position.x += xpos * 0.6;
        entities.push_back(alien);
    }
    alien.position.x=-3.5;
    for(int i=0; i < 10; i++) {
        alien.position.x += xpos * 0.6;
        alien.position.y = ypos -enemyWidth;
        entities.push_back(alien);
    }
    
    for( int i=0; i <entities.size(); i++){
        entities[i].cooldown= (float)rand() / (float)RAND_MAX * 100.0f;
    }
    for(int i=0; i < MAX_BULLETS; i++) {
        bullets[i].position.x = -2000.0f;
        bullets[i].sprite=SheetSprite(spriteSheetTexture, 856.0f/1024.0f, 421.0f/1024.0f, 9.0f/1024.0f, 54.0f / 1024.0f, 0.5);
    }

    for(int i=0; i < MAX_BULLETS; i++) {
    enemyBullets[i].position.x = -2000.0f;
    enemyBullets[i].sprite=SheetSprite(spriteSheetTexture, 856.0f/1024.0f, 421.0f/1024.0f, 9.0f/1024.0f, 54.0f / 1024.0f, 0.5);
    }
}

void reset(){
    score =0;
    for (int i=0; i<20; i++){
        entities.pop_back();
    }
    //std::cout<<entities.size()<<std::endl;
    playerWidth =(playerSprite.width/playerSprite.height)*playerSprite.size;
    enemyWidth = (alien.sprite.width/alien.sprite.height)*alien.sprite.size;
    playerX = 0.0f;
    playerY = -2.0f + enemyWidth/2;
    float xpos = 1;
    float ypos = 2.0f - enemyWidth/2;
    alien.position.x=-3.5;
    alien.position.y= 2.0f - enemyWidth/2;
    for(int i=0; i < 10; i++) {
        alien.position.x += xpos * 0.6;
        entities.push_back(alien);
    }
    alien.position.x=-3.5;
    for(int i=0; i < 10; i++) {
        alien.position.x += xpos * 0.6;
        alien.position.y = ypos -enemyWidth;
        entities.push_back(alien);
    }
    for (int i=0; i<MAX_BULLETS; i++){
        bullets[i].position.y=-2000;
        bullets[i].position.x=0;
        bullets[i].velocity.y=0;
        enemyBullets[i].position.y=2000;
        enemyBullets[i].position.x=-2000;
        enemyBullets[i].velocity.y=0;
    }
    for( int i=0; i <entities.size(); i++){
        entities[i].alive=true;
        entities[i].cooldown= (float)rand() / (float)RAND_MAX * 100.0f;
    }
}



void ProcessEvents(){
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
        else if(event.type ==SDL_KEYDOWN){
            if(event.key.keysym.scancode ==SDL_SCANCODE_SPACE){
                //do an action if spacebar is pressed
                bullets[bulletIndex].position.x = playerX;
                bullets[bulletIndex].position.y = playerY+playerWidth/2;
                bullets[bulletIndex].velocity.y= 2;
                bulletIndex++;
                if(bulletIndex > MAX_BULLETS-1) {
                    bulletIndex = 0;
                }
            }
        }
    }
}


void UpdateMainMenu(){
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if(keys[SDL_SCANCODE_RETURN]){
        state = STATE_GAME_LEVEL;
    }
    
}

void UpdateGameLevel(){
    float ticks = (float)SDL_GetTicks()/1000.0f;
    float elapsed = ticks- lastFrameTicks;
    lastFrameTicks=ticks;
    
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if(keys[SDL_SCANCODE_A]){
        if(playerX-(playerWidth/2)>=-3.55)
            playerX -= elapsed *2.0;
    }
    if(keys[SDL_SCANCODE_D]){
        if((playerX+playerWidth/2)<=3.55)
            playerX += elapsed *2.0;
    }
    
    
    for(int i=0; i < entities.size(); i++) {
        entities[i].position.x += elapsed *enemySpeed;
        if((entities[i].position.x+ enemyWidth/2)>=3.55 && entities[i].position.y < 1000.0f){
            enemySpeed = -fabs(enemySpeed);
            for(int i=0; i < entities.size(); i++)
                entities[i].position.y -= 0.2;
            
        }
        if ((entities[i].position.x- enemyWidth/2)<=-3.55 && entities[i].position.y <1000.0f){
            enemySpeed = fabs(enemySpeed);
            for(int i=0; i < entities.size(); i++)
                entities[i].position.y -= 0.2;
        }
        
    }
    
    //collision
    for(int i=0; i < MAX_BULLETS; i++) {
        bullets[i].position.y += elapsed* bullets[i].velocity.y;
        for(int j=0; j < entities.size(); j++) {
            if (bullets[i].position.y+0.175 >= entities[j].position.y - enemyWidth/2 && bullets[i].position.x<entities[j].position.x+enemyWidth/2 && bullets[i].position.x > entities[j].position.x-enemyWidth/2){
                entities[j].position.y= 100000;
                entities[j].alive=false;
                bullets[i].position.y=-2000;
                bullets[i].position.x=0;
                bullets[i].velocity.y=0;
                score+=100;
            }
        }
    }
    
    for(int i=0; i < entities.size(); i++) {
        if(entities[i].position.y-enemyWidth/2 < playerY+playerWidth/2){
             state = STATE_MAIN_MENU;
        }
    }
    if (score >= 2000){
        state = STATE_MAIN_MENU;
        reset();
    }
    
    
    //alien shoot

    for(int i=0; i < entities.size(); i++) {
        if(entities[i].alive == true){
            entities[i].cooldown -= elapsed;
            if (entities[i].cooldown <= 0.0f){
                enemyBullets[enemyBulletIndex].position.x = entities[i].position.x;
                enemyBullets[enemyBulletIndex].position.y = entities[i].position.y-enemyWidth/2;
                enemyBullets[enemyBulletIndex].velocity.y= -3.0f;
                entities[i].cooldown= (float)rand() / (float)RAND_MAX * 100.0f;
            }
            enemyBullets[enemyBulletIndex].position.y += elapsed *enemyBullets[enemyBulletIndex].velocity.y;
            enemyBulletIndex++;
            if(enemyBulletIndex > MAX_BULLETS-1) {
                enemyBulletIndex = 0;
            }
        }
    }
    
    //alien bullet collision;
    for (int i=0; i<MAX_BULLETS; i++){
        if (enemyBullets[i].position.y -0.175 < playerY+playerWidth/2 && enemyBullets[i].position.x<playerX+playerWidth/2 && enemyBullets[i].position.x > playerX-playerWidth/2 && enemyBullets[i].position.y +0.175 >playerY-playerWidth/2){
            enemyBullets[i].position.y=2000;
            enemyBullets[i].position.x=-2000;
            enemyBullets[i].velocity.y=0;
            //std::cout<<"game over"<<std::endl;
            state = STATE_MAIN_MENU;
            reset();
        }
    }
}

void RenderMainMenu(){
    glClear(GL_COLOR_BUFFER_BIT);
    modelMatrix5.identity();
    modelMatrix5.Translate(-2.5, 0.5, 0);
    program->setModelMatrix(modelMatrix5);
    program->setProjectionMatrix(projectionMatrix);
    program->setViewMatrix(viewMatrix);
    DrawText(program, fontTexture, "SPACE INVADERS", 0.4, 0.0);
    
    modelMatrix5.identity();
    modelMatrix5.Translate(-1.9, -1.0, 0);
    program->setModelMatrix(modelMatrix5);
    DrawText(program, fontTexture, "PRESS ENTER TO START", 0.2, 0.0);
    
    SDL_GL_SwapWindow(displayWindow);
    
}

void RenderGameLevel(){
    glClear(GL_COLOR_BUFFER_BIT);
    modelMatrix.identity();
    modelMatrix.Translate(playerX, playerY, 0);
    modelMatrix.Rotate(180* 3.1415/180);
    program->setModelMatrix(modelMatrix);
    program->setProjectionMatrix(projectionMatrix);
    program->setViewMatrix(viewMatrix);
    playerSprite.Draw(program);
    
    for(int i=0; i < entities.size(); i++) {
        modelMatrix2.identity();
        modelMatrix2.Translate(entities[i].position.x, entities[i].position.y, 0);
        program->setModelMatrix(modelMatrix2);
        entities[i].sprite.Draw(program);
    }
    for(int i=0; i < MAX_BULLETS; i++) {
        modelMatrix3.identity();
        modelMatrix3.Translate(bullets[i].position.x, bullets[i].position.y, 0);
        program->setModelMatrix(modelMatrix3);
        bullets[i].sprite.Draw(program);
    }
    
    for(int i=0; i < MAX_BULLETS; i++) {
        modelMatrix4.identity();
        modelMatrix4.Translate(enemyBullets[i].position.x, enemyBullets[i].position.y, 0);
        modelMatrix4.Rotate(180* 3.1415/180);
        program->setModelMatrix(modelMatrix4);
        enemyBullets[i].sprite.Draw(program);
    }
    
    modelMatrix6.identity();
    modelMatrix6.Translate(-3.46, -1.85, 0);
    program->setModelMatrix(modelMatrix6);
    DrawText(program, fontTexture, std::to_string(score) , 0.2, 0.0);
    SDL_GL_SwapWindow(displayWindow);
}

void Render() {
    switch(state) {
        case STATE_MAIN_MENU:
            RenderMainMenu();
            break;
        case STATE_GAME_LEVEL:
            RenderGameLevel();
            break;
    }
}

void Update(){
    switch(state) {
        case STATE_MAIN_MENU:
            UpdateMainMenu();
            break;
        case STATE_GAME_LEVEL:
            UpdateGameLevel();
            break;
    }
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





