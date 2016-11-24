#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

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
    Vector3 acceleration;
    Vector3 friction;
    float rotation;
    bool collidBot = false;
    bool collidTop = false;
    bool collidLeft = false;
    bool collidRight = false;
};

SDL_Window* displayWindow;
ShaderProgram *program;
Matrix projectionMatrix; // aspect ratio
Matrix modelMatrix;
Matrix modelMatrix2;
Matrix modelMatrix3;
Matrix viewMatrix;
Matrix viewMatrix2;
SDL_Event event;
Mix_Chunk *someSound;
Mix_Music *music;
float lastFrameTicks= 0.0f;
bool done = false;
GLuint mapTexture;
GLuint fontTexture;
enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL };
int state;
int score=0;
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
#define TILE_SIZE 16.0f
#define SPRITE_COUNT_X 16
#define SPRITE_COUNT_Y 8
#define GRAVITY -60
int mapWidth;
int mapHeight;
unsigned char **levelData;
Entity player;
Entity enemy;
bool solid;
float playerX;
float playerY;
float enemyX;
float enemyY;



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


void draw(ShaderProgram *program, int mapTexture){
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    int drawCount =0;
    for(int y=0; y < mapHeight; y++) {
        for(int x=0; x < mapWidth; x++) {
        if(levelData[y][x] != 0) {
            drawCount++;
            float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float) SPRITE_COUNT_X;
            float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float) SPRITE_COUNT_Y;
            float spriteWidth = 1.0f/(float)SPRITE_COUNT_X;
            float spriteHeight = 1.0f/(float)SPRITE_COUNT_Y;
            vertexData.insert(vertexData.end(), {
                TILE_SIZE * x, -TILE_SIZE * y,
                TILE_SIZE * x, (-TILE_SIZE * y)-TILE_SIZE,
                (TILE_SIZE * x)+TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,
                TILE_SIZE * x, -TILE_SIZE * y,
                (TILE_SIZE * x)+TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,
                (TILE_SIZE * x)+TILE_SIZE, -TILE_SIZE * y
            });
            texCoordData.insert(texCoordData.end(), {
                u, v,
                u, v+(spriteHeight),
                u+spriteWidth, v+(spriteHeight),
                u, v,
                u+spriteWidth, v+(spriteHeight),
                u+spriteWidth, v
            });
        }
        }
    }
    glUseProgram(program->programID);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, mapTexture);
    glDrawArrays(GL_TRIANGLES, 0, (int)((drawCount) * 6));
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void drawEntity(ShaderProgram *program, int mapTexture, int index){
    float u = (float)(((int)index) % SPRITE_COUNT_X) / (float) SPRITE_COUNT_X;
    float v = (float)(((int)index) / SPRITE_COUNT_X) / (float) SPRITE_COUNT_Y;
    float spriteWidth = 1.0/(float)SPRITE_COUNT_X;
    float spriteHeight = 1.0/(float)SPRITE_COUNT_Y;
    GLfloat texCoords[] = {
        u, v+spriteHeight,
        u+spriteWidth, v,
        u, v,
        u+spriteWidth, v,
        u, v+spriteHeight,
        u+spriteWidth, v+spriteHeight
    };
    float vertices[] = {
        -TILE_SIZE/2, -TILE_SIZE/2,
        TILE_SIZE/2, TILE_SIZE/2,
        -TILE_SIZE/2, TILE_SIZE/2,
        TILE_SIZE/2, TILE_SIZE/2,
        -TILE_SIZE/2, -TILE_SIZE/2,
        TILE_SIZE/2, -TILE_SIZE/2
    };

    glUseProgram(program->programID);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, mapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
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




bool readHeader(std::ifstream &stream) {
    string line;
    //mapWidth = -1;
    //mapHeight = -1;
    while(getline(stream, line)) {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "width") {
            mapWidth = atoi(value.c_str());
        } else if(key == "height"){
            mapHeight = atoi(value.c_str());
        } }
    if(mapWidth == -1 || mapHeight == -1) {
        return false;
    } else { // allocate our map data
        levelData = new unsigned char*[mapHeight];
        for(int i = 0; i < mapHeight; ++i) {
            levelData[i] = new unsigned char[mapWidth];
        }
        return true;
    }
}

bool readLayerData(std::ifstream &stream) {
    string line;
    while(getline(stream, line)) {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "data") {
            for(int y=0; y < mapHeight; y++) {
                getline(stream, line);
                istringstream lineStream(line);
                string tile;
                for(int x=0; x < mapWidth; x++) {
                    getline(lineStream, tile, ',');
                    unsigned char val =  (unsigned char)atoi(tile.c_str());
                    if(val > 0) {
                        // be careful, the tiles in this format are indexed from 1 not 0
                        levelData[y][x] = val-1;
                    } else {
                        levelData[y][x] = 0;
                    }
                }
            }
        }
    }
     return true;
}

void placeEntity(string type, float x, float y){
    if (type == "player"){
        player.position.x = x;
        player.position.y = y;
        playerX = x;
        playerY=y;
    }
    if (type == "enemy"){
        enemy.position.x = x;
        enemy.position.y = y;
        enemyX=x;
        enemyY=y;
    }
}

bool readEntityData(std::ifstream &stream) {
    string line;
    string type;
    while(getline(stream, line)) {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "type") {
            type = value;
        } else if(key == "location") {
            istringstream lineStream(value);
            string xPosition, yPosition;
            getline(lineStream, xPosition, ',');
            getline(lineStream, yPosition, ',');
                    float placeX = atoi(xPosition.c_str())*TILE_SIZE;
                    float placeY = atoi(yPosition.c_str())*-TILE_SIZE;
                    placeEntity(type, placeX, placeY);
                    }
                    }
                    return true;
}


float lerp(float v0, float v1, float t) {
    return (1.0-t)*v0 + t*v1;
}

bool isSolid(int x, int y){
    if(levelData[y][x] == 1 || levelData[y][x] == 2 || levelData[y][x] == 3 || levelData[y][x] == 4) {
        return true;
    }
    else
        return false;
}



void collisionY(Entity *e){
    int mapX = (int)(e->position.x/TILE_SIZE);
    int mapYBot = (int)((-(e->position.y-TILE_SIZE/2))/TILE_SIZE);
    int mapYTop =(int)((-(e->position.y+TILE_SIZE/2))/TILE_SIZE);
    if (mapYBot <= mapHeight-1 && mapYBot >=0 && mapX >= 0 && mapX <= mapWidth-1 && mapYTop <=mapHeight-1 && mapYTop>=0){
        if(isSolid(mapX,mapYBot) == true){
            if ((e->position.y-TILE_SIZE/2) <= (-TILE_SIZE* mapYBot)){
                e->collidBot = true;
                float penetration = fabs((-TILE_SIZE* mapYBot)-(e->position.y-TILE_SIZE/2));
                e->position.y = e->position.y + penetration +0.001;
                e->velocity.y=0;
            }
        }
        if(isSolid(mapX,mapYTop) == true){
            if ((e->position.y+TILE_SIZE/2) >= ((-TILE_SIZE* mapYTop)-TILE_SIZE)){
                float penetration = fabs((e->position.y+TILE_SIZE/2)-((-TILE_SIZE* mapYTop)-TILE_SIZE));
                e->position.y = e->position.y - penetration - 0.001;
                e->collidTop = true;
            }
        }
    }
}

void collisionX(Entity *e){
    int mapXLeft= (int)(((e->position.x-TILE_SIZE/2))/TILE_SIZE);;
    int mapXRight= (int)(((e->position.x+TILE_SIZE/2))/TILE_SIZE);;
    int mapY = (int)(-e->position.y/TILE_SIZE);
    if (mapXLeft <= mapWidth-1 && mapXLeft >=0 && mapY >= 0 && mapY <= mapHeight-1 && mapXRight <=mapWidth-1 && mapXRight>=0){
        if(isSolid(mapXLeft,mapY) == true){
            if ((e->position.x-TILE_SIZE/2) <= ((TILE_SIZE* mapXLeft)+TILE_SIZE)){
                e->collidLeft = true;
                float penetration = fabs((e->position.x-TILE_SIZE/2)-((TILE_SIZE* mapXLeft)+TILE_SIZE));
                e->position.x = e->position.x + penetration +0.001;
                e->velocity.x=0;
            }
        }
        if(isSolid(mapXRight,mapY) == true){
            if ((e->position.x+TILE_SIZE/2) >= (TILE_SIZE* mapXRight)){
                e->collidRight = true;
                float penetration = fabs((TILE_SIZE* mapXRight)-(e->position.x+TILE_SIZE/2));
                e->position.x = e->position.x - penetration - 0.001;
                e->velocity.x=0;
            }
            
        }
    }
}


void enemyCollisionX(){
    int mapXLeft= (int)(((enemy.position.x-TILE_SIZE/2))/TILE_SIZE);;
    int mapXRight= (int)(((enemy.position.x+TILE_SIZE/2))/TILE_SIZE);;
    int mapY = (int)(-enemy.position.y/TILE_SIZE);
    if (mapXLeft <= mapWidth-1 && mapXLeft >=0 && mapY >= 0 && mapY <= mapHeight-1 && mapXRight <=mapWidth-1 && mapXRight>=0){
        if(isSolid(mapXLeft,mapY) == true){
            if ((enemy.position.x-TILE_SIZE/2) <= ((TILE_SIZE* mapXLeft)+TILE_SIZE)){
                enemy.collidLeft = true;
                float penetration = fabs((enemy.position.x-TILE_SIZE/2)-((TILE_SIZE* mapXLeft)+TILE_SIZE));
                enemy.position.x = enemy.position.x + penetration +0.001;
                enemy.velocity.x= -(enemy.velocity.x);
            }
        }
        if(isSolid(mapXRight,mapY) == true){
            if ((enemy.position.x+TILE_SIZE/2) >= (TILE_SIZE* mapXRight)){
                enemy.collidRight = true;
                float penetration = fabs((TILE_SIZE* mapXRight)-(enemy.position.x+TILE_SIZE/2));
                enemy.position.x = enemy.position.x - penetration - 0.001;
                enemy.velocity.x= -(enemy.velocity.x);
                
            }
            
        }
    }
}

void playerEnemyCollision(){
    float pTop = player.position.y + TILE_SIZE/2;
    float pBot = player.position.y - TILE_SIZE/2;
    float pLeft = player.position.x - TILE_SIZE/2;
    float pRight = player.position.x +TILE_SIZE/2;
    float eTop = enemy.position.y + TILE_SIZE/2;
    float eBot = enemy.position.y - TILE_SIZE/2;
    float eLeft = enemy.position.x - TILE_SIZE/2;
    float eRight = enemy.position.x +TILE_SIZE/2;
    if (!(pBot > eTop) && !(pTop < eBot) && !(pLeft>eRight) && !(pRight < eLeft) ){
        enemy.position.x = 10000;
        enemy.position.y=10000;
    }
    
}

void reset(){
    player.position.x = playerX;
    player.position.y = playerY;
    player.velocity.x=0;
    player.velocity.y=0;
    enemy.position.x = enemyX;
    enemy.position.y = enemyY;
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
    glUseProgram(program->programID);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    mapTexture = LoadTexture("arne_sprites.png");
    fontTexture= LoadTexture("font1.png");
    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 );
    someSound = Mix_LoadWAV("jump.wav");
    music = Mix_LoadMUS("music.mp3");
    Mix_PlayMusic(music, -1);
    ifstream infile("NYUCodebase.app/Contents/Resources/game.txt");
    string line;
    while (getline(infile, line)) {
        // handle line
        if(line == "[header]") {
            if(!readHeader(infile)) {
                return;
        }
        } else if(line == "[layer]") {
            readLayerData(infile);
        } else if(line == "[Object Layer 1]") {
            readEntityData(infile);
        }
    }
    player.friction.x= 2.5;
    player.friction.y= 0.1;
    enemy.velocity.x =15;
}



void ProcessEvents(){
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
        else if(event.type ==SDL_KEYDOWN){
            if(event.key.keysym.scancode ==SDL_SCANCODE_SPACE){
                //do an action if spacebar is pressed
            }
        }
    }
}


// game level updates
void fixUpdate(float elapsed){
    
    player.collidBot= false;
    player.collidTop=false;
    player.collidLeft=false;
    player.collidRight=false;
    
    player.velocity.x = lerp(player.velocity.x, 0.0f, elapsed * player.friction.x);
    player.velocity.y = lerp(player.velocity.y, 0.0f, elapsed * player.friction.y);
    player.velocity.y += (GRAVITY) * elapsed;
    
    
    player.position.y +=  player.velocity.y * elapsed;
    //collision y
    collisionY(&player);
    
    player.position.x +=  player.velocity.x * elapsed;
    //collision X
    collisionX(&player);
    
    enemy.velocity.y += (GRAVITY) * elapsed;
    enemy.position.y +=  enemy.velocity.y * elapsed;
    //enemy collision y
    collisionY(&enemy);
    
    enemy.position.x +=  enemy.velocity.x * elapsed;
    //enemy collision x
    enemyCollisionX();
    
    // player enemy Collision
    playerEnemyCollision();
    
    
    //reset
    if (player.position.y < -mapHeight*TILE_SIZE-100){
        state = STATE_MAIN_MENU;
        reset();
    }
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if(keys[SDL_SCANCODE_W]){
        if(player.collidBot == true){
            //booster
            //player.velocity.y += player.acceleration.y * elapsed;
            //player.acceleration.y= 2700;
            Mix_PlayChannel( -1, someSound, 0);
            player.velocity.y=50;
        }
        
    }
    if(keys[SDL_SCANCODE_A]){
        player.velocity.x += player.acceleration.x * elapsed;
        if(player.acceleration.x==0){
            player.acceleration.x=-110;
        }
        else{
            player.acceleration.x= -fabs(player.acceleration.x);
        }
    }
    if(keys[SDL_SCANCODE_D]){
        player.velocity.x += player.acceleration.x * elapsed;
        if(player.acceleration.x==0){
            player.acceleration.x=110;
        }
        else{
            player.acceleration.x= fabs(player.acceleration.x);
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



void RenderMainMenu(){
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    projectionMatrix.setOrthoProjection(-3.0f, 3.0f, -2.0f, 2.0f, -1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    modelMatrix.identity();
    modelMatrix.Translate(-1.8, 0.5, 0);
    program->setModelMatrix(modelMatrix);
    program->setProjectionMatrix(projectionMatrix);
    program->setViewMatrix(viewMatrix);
    DrawText(program, fontTexture, "Platformer", 0.4, 0.0);
    modelMatrix.identity();
    modelMatrix.Translate(-1.9, -1.0, 0);
    program->setModelMatrix(modelMatrix);
    DrawText(program, fontTexture, "PRESS ENTER TO START", 0.2, 0.0);
    
    SDL_GL_SwapWindow(displayWindow);
}

void RenderGameLevel(){
    glClearColor(0.0f, 0.7f, 0.9f, 1.0f);
    projectionMatrix.setOrthoProjection(-200.0f, 200.0f, -80.0f, 80.0f, -1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    modelMatrix2.identity();
    program->setModelMatrix(modelMatrix2);
    program->setProjectionMatrix(projectionMatrix);
    draw(program,mapTexture);
    modelMatrix2.identity();
    viewMatrix2.identity();
    modelMatrix2.Translate(player.position.x, player.position.y, 0.0f);
    viewMatrix2.Translate(-(player.position.x), -(player.position.y), 0.0f);
    program->setModelMatrix(modelMatrix2);
    program->setViewMatrix(viewMatrix2);
    drawEntity(program, mapTexture, 98);
    modelMatrix3.identity();
    modelMatrix3.Translate(enemy.position.x, enemy.position.y, 0.0f);
    program->setModelMatrix(modelMatrix3);
    drawEntity(program, mapTexture, 80);
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
    Mix_FreeChunk(someSound);
    Mix_FreeMusic(music);
    SDL_Quit();
    return 0;
}





