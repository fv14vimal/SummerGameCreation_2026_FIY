#include "Game.h"
#include "DxPlus/DxPlus.h"
#include "WinMain.h"
#include "Entity2D.h"


constexpr float MOVE_SPEED = 3.0f;



extern int nextScene;
int gameState;
float gameFadeTimer;

int playerID;                  
int backID;                    
Entity2D player;              
bool wasPressed = false;        
bool isMovingRight = true;      


void Game_Init()
{
    DxLib::SetBackgroundColor(0, 0, 0);
    playerID = DxPlus::Sprite::Load(L"./Data/Images/mouse.png");
    backID = DxPlus::Sprite::Load(L"./Data/Images/background.png");
    Game_Reset();
}


void ResetPlayerToStart()
{
    player.position = { DxPlus::CLIENT_WIDTH / 2.0f, DxPlus::CLIENT_HEIGHT / 2.0f };
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

    player.position.x += player.velocity.x;
}


void Game_Render()
{
   

    constexpr DxPlus::Vec2 BG_SCALE = { 1.6f, 1.6f };
    constexpr DxPlus::Vec2 BG_CENTER = { 0.0f, 0.0f };
    DxPlus::Sprite::Draw(backID,{ 0.0f, 0.0f },BG_SCALE,BG_CENTER);


    if (player.isActive)
    {
        constexpr DxPlus::Vec2 PLAYER_SCALE = { 0.15f, 0.15f };
        constexpr DxPlus::Vec2 PLAYER_CENTER = { 0.5f, 0.5f };
        DxPlus::Sprite::Draw(player.spriteID,player.position,PLAYER_SCALE,PLAYER_CENTER);
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