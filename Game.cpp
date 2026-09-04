#include "Game.h"
#include "DxPlus/DxPlus.h"
#include "WinMain.h"
#include "Entity2D.h"
#include <cmath>

constexpr float MOVE_SPEED = 3.0f;
constexpr float PLAYER_SCALE_MAG = 0.05f; 
constexpr float WALL_HALF_THICKNESS = 1.5f; 


struct WallRect
{
    DxPlus::Vec2 leftTop;
    DxPlus::Vec2 rightBottom;
};

constexpr float WALL_X[] = { 427.0f, 854.0f };
constexpr int WALL_COUNT = sizeof(WALL_X) / sizeof(WALL_X[0]);
static WallRect wallRects[WALL_COUNT];

extern int nextScene;
int gameState;
float gameFadeTimer;
int playerID;
int backID;
Entity2D player;
bool wasPressed = false;
bool isMovingRight = true;


static DxPlus::Vec2 playerCenterPx = { 0.0f, 0.0f };
static float playerRadius = 0.0f; 

void Game_Init()
{
    DxLib::SetBackgroundColor(0, 0, 0);
    playerID = DxPlus::Sprite::Load(L"./Data/Images/mouse.png");
    backID = DxPlus::Sprite::Load(L"./Data/Images/background.png");

    int imgW = 0, imgH = 0;
    DxLib::GetGraphSize(playerID, &imgW, &imgH);
    playerCenterPx = { imgW * 0.5f, imgH * 0.5f };
    playerRadius = imgW * PLAYER_SCALE_MAG * 0.5f;

    
    for (int i = 0; i < WALL_COUNT; ++i)
    {
        wallRects[i].leftTop = { WALL_X[i] - WALL_HALF_THICKNESS, 0.0f };
        wallRects[i].rightBottom = { WALL_X[i] + WALL_HALF_THICKNESS, static_cast<float>(DxPlus::CLIENT_HEIGHT) };
    }

    Game_Reset();
}

void ResetPlayerToStart()
{
    player.position = { DxPlus::CLIENT_WIDTH / 35.0f, DxPlus::CLIENT_HEIGHT / 25.0f };
    player.velocity = { 0, 0 };
    player.spriteID = playerID;
    player.isActive = true;
}

void Game_Reset()
{
    gameState = 0;
    gameFadeTimer = 1.0f;
    isMovingRight = true;
    wasPressed = false;
    ResetPlayerToStart();
}

void HandleInput()
{
    int keyState = DxLib::CheckHitKey(KEY_INPUT_SPACE);
    if (keyState && !wasPressed)
    {
        isMovingRight = !isMovingRight;
    }
    wasPressed = keyState;
    player.velocity.x = keyState ? (isMovingRight ? MOVE_SPEED : -MOVE_SPEED) : 0.0f;
}


void ResolveWallCollisions(float prevX)
{
    for (int i = 0; i < WALL_COUNT; ++i)
    {
        const WallRect& rect = wallRects[i];

       
        float closestX = player.position.x;
        if (closestX < rect.leftTop.x) closestX = rect.leftTop.x;
        else if (closestX > rect.rightBottom.x) closestX = rect.rightBottom.x;

        
        float closestY = player.position.y;
        if (closestY < rect.leftTop.y) closestY = rect.leftTop.y;
        else if (closestY > rect.rightBottom.y) closestY = rect.rightBottom.y;

        float dx = player.position.x - closestX;
        float dy = player.position.y - closestY;
        float distSq = dx * dx + dy * dy;

        if (distSq < playerRadius * playerRadius)
        {
            
            if (prevX <= WALL_X[i])
            {
                player.position.x = rect.leftTop.x - playerRadius;
            }
            else
            {
                player.position.x = rect.rightBottom.x + playerRadius;
            }
            player.velocity.x = 0.0f;
        }
    }
}

void Game_Update()
{
    switch (gameState)
    {
    case 0:
    {
        gameFadeTimer -= 1 / 60.0f;
        if (gameFadeTimer < 0.0f)
        {
            gameFadeTimer = 0.0f;
            gameState++;
        }
        break;
    }
    case 1:
    {
        Game_Play();
        int input = DxPlus::Input::GetButtonDown(DxPlus::Input::PLAYER1);
        if (input & DxPlus::Input::BUTTON_SELECT)
        {
            gameState++;
        }
        break;
    }
    case 2:
    {
        gameFadeTimer += 1 / 60.0f;
        if (gameFadeTimer > 1.0f)
        {
            gameFadeTimer = 1.0f;
            nextScene = SceneTitle;
        }
        break;
    }
    }
}

void Game_Play()
{
    HandleInput();
    float prevX = player.position.x;
    player.position.x += player.velocity.x;
    ResolveWallCollisions(prevX);

    //書き足した端の当たり判定
    if (player.position.x < playerRadius)
    {
        player.position.x = playerRadius;
    }
    if (player.position.x > DxPlus::CLIENT_WIDTH - playerRadius)
    {
        player.position.x = DxPlus::CLIENT_WIDTH - playerRadius;
    }
}

void Game_Render()
{
    constexpr DxPlus::Vec2 BG_SCALE = { 1.6f, 1.6f };
    constexpr DxPlus::Vec2 BG_CENTER = { 0.0f, 0.0f };
    DxPlus::Sprite::Draw(backID, { 0.0f, 0.0f }, BG_SCALE, BG_CENTER);
    DxPlus::Primitive2D::DrawLine({ 427, 0 }, { 427, 720 }, DxLib::GetColor(0, 0, 0), 3.0f);
    DxPlus::Primitive2D::DrawLine({ 854, 0 }, { 854, 720 }, DxLib::GetColor(0, 0, 0), 3.0f);
    DxPlus::Primitive2D::DrawLine({ 0, 0 }, { 0, 720 }, DxLib::GetColor(0, 0, 0), 3.0f);
    DxPlus::Primitive2D::DrawLine({ 1280, 0 }, { 1280, 720 }, DxLib::GetColor(0, 0, 0), 3.0f);

    if (player.isActive)
    {
        float scaleX = isMovingRight ? -PLAYER_SCALE_MAG : PLAYER_SCALE_MAG;
        DxPlus::Vec2 playerScale = { scaleX, PLAYER_SCALE_MAG };
        DxPlus::Sprite::Draw(player.spriteID, player.position, playerScale, playerCenterPx);
    }

    if (gameFadeTimer > 0.0f)
    {
        DxLib::SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * gameFadeTimer));
        DxPlus::Primitive2D::DrawRect({ 0,0 },
            { DxPlus::CLIENT_WIDTH, DxPlus::CLIENT_HEIGHT }, DxLib::GetColor(0, 0, 0));
        DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void Game_End()
{

}