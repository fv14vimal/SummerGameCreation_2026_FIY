#include "Game.h"
#include "DxPlus/DxPlus.h"
#include "WinMain.h"
#include "Entity2D.h"
#include <cmath>
#include "GameClear.h"
#include"GameOrve.h"

constexpr float MOVE_SPEED = 3.0f;
constexpr float PLAYER_SCALE_MAG = 0.05f;
constexpr float WALL_HALF_THICKNESS = 1.5f;
constexpr float FLOOR_SNAP_TOLERANCE = 5.0f;
constexpr DxPlus::Vec2 ARROW_POSITION = { 1180.0f, 60.0f };

constexpr float END_ZONE_X = 1150.0f;

// バネ・スイッチ・ゲートの「見た目の目標幅」。実際の画像サイズから逆算して倍率を決める
constexpr float SPRING_TARGET_WIDTH = 60.0f;
constexpr float SWITCH_TARGET_WIDTH = 55.0f;
constexpr float GATE_TARGET_WIDTH = 130.0f; // 背景のトンネル穴に合わせたサイズ

constexpr float SPRING_LAUNCH_VELOCITY = -30.0f; // 要調整
constexpr DxPlus::Vec2 SPRING_POSITION = { 970.0f, 200.0f };
constexpr DxPlus::Vec2 SWITCH_POSITION = { 970.0f, 120.0f };

// ゲート：背景に描かれているトンネル穴の位置に合わせる
constexpr DxPlus::Vec2 GATE_POSITIONS[] =
{
    { 52.0f, 32.0f },    // スタート地点のトンネル
    { 454.0f, 40.0f },   // セクション1→2 着地トンネル
    { 916.0f, 40.0f }   // セクション2→3 着地トンネル
};
constexpr int GATE_COUNT = sizeof(GATE_POSITIONS) / sizeof(GATE_POSITIONS[0]);

struct WallRect
{
    DxPlus::Vec2 leftTop;
    DxPlus::Vec2 rightBottom;
};

struct Line
{
    DxPlus::Vec2 start;
    DxPlus::Vec2 end;
};

struct SectionTransition
{
    DxPlus::Vec2 destination;
};

struct MovableFloor
{
    Line line;
    bool isOpen;
};

constexpr Line sideLines[] =
{
    //SIDE1
    {{0,240},{300,240}},
    {{150,440},{427,440}},

    //SIDE2
    {{427,140},{700,140}},
    {{550,290},{854,290}},
    {{427,400},{600,400}},
    {{550,550},{854,550}},

    //SIDE3
    {{854,240},{1100,240}},
    {{854,400},{1000,400}},
    {{1150,400},{1280,400}},
    {{1024,550},{1280,550}}
};
constexpr int SIDE_LINE_COUNT =
sizeof(sideLines) / sizeof(sideLines[0]);
constexpr float WALL_X[] = { 427.0f, 854.0f };
constexpr int WALL_COUNT = sizeof(WALL_X) / sizeof(WALL_X[0]);
static WallRect wallRects[WALL_COUNT];

static MovableFloor secondFloorGap = { {{1000.0f, 400.0f}, {1150.0f, 400.0f}}, false };

constexpr SectionTransition sectionTransitions[WALL_COUNT] =
{
    { {460.0f, 20.0f} },
    { {900.0f, 20.0f} }
};

extern int nextScene;
int gameState;
float gameFadeTimer;
int playerID;
int arrowID;
int backID;
int gateID;
int springID;
int switchOffID;
int switchOnID;
int cheeseID;
Entity2D player;
bool wasPressed = false;
bool isMovingRight = true;
bool reachedEnd = false;
bool switchActivated = false;

static DxPlus::Vec2 playerCenterPx = { 0.0f, 0.0f };
static DxPlus::Vec2 arrowCenterPx = { 0.0f, 0.0f };
static DxPlus::Vec2 gateCenterPx = { 0.0f, 0.0f };
static DxPlus::Vec2 springCenterPx = { 0.0f, 0.0f };
static DxPlus::Vec2 switchCenterPx = { 0.0f, 0.0f };
static float playerRadius = 0.0f;
static float springRadius = 0.0f;
static float switchRadius = 0.0f;
static float gateScale = 1.0f;
static float springScale = 1.0f;
static float switchScale = 1.0f;

void Game_Init()
{
    DxLib::SetBackgroundColor(0, 0, 0);
    playerID = DxPlus::Sprite::Load(L"./Data/Images/mouse.png");
    arrowID = DxPlus::Sprite::Load(L"./Data/Images/arrow.png");
    backID = DxPlus::Sprite::Load(L"./Data/Images/background.png");
    gateID = DxPlus::Sprite::Load(L"./Data/Images/gate.png");
    springID = DxPlus::Sprite::Load(L"./Data/Images/spring.png");
    switchOffID = DxPlus::Sprite::Load(L"./Data/Images/switch_off.png");
    switchOnID = DxPlus::Sprite::Load(L"./Data/Images/switch_on.png");
    cheeseID = DxPlus::Sprite::Load(L"./Data/Images/cheese.png");

    int imgW = 0, imgH = 0;
    DxLib::GetGraphSize(playerID, &imgW, &imgH);
    playerCenterPx = { imgW * 0.5f, imgH * 0.5f };
    playerRadius = imgW * PLAYER_SCALE_MAG * 0.5f;

    int arrowW = 0, arrowH = 0;
    DxLib::GetGraphSize(arrowID, &arrowW, &arrowH);
    arrowCenterPx = { arrowW * 0.5f, arrowH * 0.5f };

    // ゲート：実際の画像幅から、目標の見た目サイズになる倍率を逆算する
    int gateW = 0, gateH = 0;
    DxLib::GetGraphSize(gateID, &gateW, &gateH);
    gateCenterPx = { gateW * 0.5f, gateH * 0.5f };
    gateScale = (gateW > 0) ? (GATE_TARGET_WIDTH / gateW) : 1.0f;

    // バネ
    int springW = 0, springH = 0;
    DxLib::GetGraphSize(springID, &springW, &springH);
    springCenterPx = { springW * 0.5f, springH * 0.5f };
    springScale = (springW > 0) ? (SPRING_TARGET_WIDTH / springW) : 1.0f;
    springRadius = springW * springScale * 0.5f;

    // スイッチ(OFF画像のサイズを基準にする。ON画像も同サイズ想定)
    int switchW = 0, switchH = 0;
    DxLib::GetGraphSize(switchOffID, &switchW, &switchH);
    switchCenterPx = { switchW * 0.5f, switchH * 0.5f };
    switchScale = (switchW > 0) ? (SWITCH_TARGET_WIDTH / switchW) : 1.0f;
    switchRadius = switchW * switchScale * 0.5f;

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
    reachedEnd = false;
    switchActivated = false;
    secondFloorGap.isOpen = false;
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
    float screenFloorPlayerY = DxPlus::CLIENT_HEIGHT - playerRadius;

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
            if (std::fabs(player.position.y - screenFloorPlayerY) < FLOOR_SNAP_TOLERANCE)
            {
                const SectionTransition& transition = sectionTransitions[i];
                player.position = transition.destination;
                player.velocity = { 0.0f, 0.0f };
                continue;
            }

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

void ResolveFloorCollisions(float prevY)
{
    for (int i = 0; i < SIDE_LINE_COUNT; ++i)
    {
        const Line& line = sideLines[i];

        if (player.position.x + playerRadius >= line.start.x &&
            player.position.x - playerRadius <= line.end.x)
        {
            if (prevY + playerRadius <= line.start.y &&
                player.position.y + playerRadius >= line.start.y)
            {
                player.position.y = line.start.y - playerRadius;
                player.velocity.y = 0.0f;
            }
        }
    }

    if (!secondFloorGap.isOpen)
    {
        const Line& line = secondFloorGap.line;
        if (player.position.x + playerRadius >= line.start.x &&
            player.position.x - playerRadius <= line.end.x)
        {
            if (prevY + playerRadius <= line.start.y &&
                player.position.y + playerRadius >= line.start.y)
            {
                player.position.y = line.start.y - playerRadius;
                player.velocity.y = 0.0f;
            }
        }
    }
}

void CheckSpring()
{
    float dx = player.position.x - SPRING_POSITION.x;
    float dy = player.position.y - SPRING_POSITION.y;
    float distSq = dx * dx + dy * dy;
    float radiusSum = playerRadius + springRadius;

    if (player.velocity.y > 0.0f && distSq < radiusSum * radiusSum)
    {
        player.velocity.y = SPRING_LAUNCH_VELOCITY;
    }
}

void CheckSwitch()
{
    if (switchActivated) return;

    float dx = player.position.x - SWITCH_POSITION.x;
    float dy = player.position.y - SWITCH_POSITION.y;
    float distSq = dx * dx + dy * dy;
    float radiusSum = playerRadius + switchRadius;

    if (distSq < radiusSum * radiusSum)
    {
        switchActivated = true;
        secondFloorGap.isOpen = true;
    }
}

void CheckEnding()
{
    if (reachedEnd) return;

    float screenFloorPlayerY = DxPlus::CLIENT_HEIGHT - playerRadius;
    if (player.position.x > END_ZONE_X &&
        std::fabs(player.position.y - screenFloorPlayerY) < FLOOR_SNAP_TOLERANCE)
    {
        reachedEnd = true;
        gameState = 2;
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
            nextScene = reachedEnd ? SceneGameClear : SceneTitle;
        }
        break;
    }
    }
}

void Game_Play()
{
    HandleInput();
    float prevX = player.position.x;
    float prevY = player.position.y;
    player.position.x += player.velocity.x;
    player.position.y += 3.5f;
    ResolveWallCollisions(prevX);
    ResolveFloorCollisions(prevY);
    CheckSpring();
    CheckSwitch();

    if (player.position.x < playerRadius)
    {
        player.position.x = playerRadius;
    }
    if (player.position.x > DxPlus::CLIENT_WIDTH - playerRadius)
    {
        player.position.x = DxPlus::CLIENT_WIDTH - playerRadius;
    }

    if (player.position.y < playerRadius)
    {
        player.position.y = playerRadius;
    }
    if (player.position.y > DxPlus::CLIENT_HEIGHT - playerRadius)
    {
        player.position.y = DxPlus::CLIENT_HEIGHT - playerRadius;
    }

    CheckEnding();
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
    DxPlus::Primitive2D::DrawLine({ 0, 0 }, { 1280, 0 }, DxLib::GetColor(0, 0, 0), 3.0f);
    DxPlus::Primitive2D::DrawLine({ 0, 720 }, { 1280, 720 }, DxLib::GetColor(0, 0, 0), 3.0f);

    //SIDE 1
    DxPlus::Primitive2D::DrawLine({ 0, 240 }, { 300, 240 }, DxLib::GetColor(194, 29, 17), 5.0f);
    DxPlus::Primitive2D::DrawLine({ 150, 440 }, { 427, 440 }, DxLib::GetColor(194, 29, 17), 5.0f);

    //SIDE 2
    DxPlus::Primitive2D::DrawLine({ 427, 140 }, { 700, 140 }, DxLib::GetColor(194, 29, 17), 5.0f);
    DxPlus::Primitive2D::DrawLine({ 550, 290 }, { 854, 290 }, DxLib::GetColor(194, 29, 17), 5.0f);
    DxPlus::Primitive2D::DrawLine({ 427, 400 }, { 600, 400 }, DxLib::GetColor(194, 29, 17), 5.0f);
    DxPlus::Primitive2D::DrawLine({ 550, 550 }, { 854, 550 }, DxLib::GetColor(194, 29, 17), 5.0f);

    //SIDE 3
    DxPlus::Primitive2D::DrawLine({ 854, 240 }, { 1100, 240 }, DxLib::GetColor(194, 29, 17), 5.0f);
    DxPlus::Primitive2D::DrawLine({ 854, 400 }, { 1000, 400 }, DxLib::GetColor(194, 29, 17), 5.0f);
    if (!secondFloorGap.isOpen)
    {
        DxPlus::Primitive2D::DrawLine({ 1000, 400 }, { 1150, 400 }, DxLib::GetColor(194, 29, 17), 5.0f);
    }
    DxPlus::Primitive2D::DrawLine({ 1150, 400 }, { 1280, 400 }, DxLib::GetColor(194, 29, 17), 5.0f);
    DxPlus::Primitive2D::DrawLine({ 1024, 550 }, { 1280, 550 }, DxLib::GetColor(194, 29, 17), 5.0f);

    // ゲート(各トンネルの位置に合わせて描画)
    for (int i = 0; i < GATE_COUNT; ++i)
    {
        DxPlus::Sprite::Draw(gateID, GATE_POSITIONS[i], { gateScale, gateScale }, gateCenterPx);
    }

    //ゴールチーズの描画
    DxPlus::Sprite::Draw(cheeseID, { 1157.0f, 683.0f } , { gateScale, gateScale }, gateCenterPx);

    // バネ
    DxPlus::Sprite::Draw(springID, SPRING_POSITION, { springScale, springScale }, springCenterPx);

    // スイッチ
    int currentSwitchID = switchActivated ? switchOnID : switchOffID;
    DxPlus::Sprite::Draw(currentSwitchID, SWITCH_POSITION, { switchScale, switchScale }, switchCenterPx);

    if (player.isActive)
    {
        float scaleX = isMovingRight ? -PLAYER_SCALE_MAG : PLAYER_SCALE_MAG;
        DxPlus::Vec2 playerScale = { scaleX, PLAYER_SCALE_MAG };
        DxPlus::Sprite::Draw(player.spriteID, player.position, playerScale, playerCenterPx);

        float arrowRotation = isMovingRight ? DxPlus::Deg2Rad * 180.0f : 0.0f;
        DxPlus::Sprite::Draw(arrowID, ARROW_POSITION, { 0.05f, 0.05f }, arrowCenterPx, arrowRotation);
    }

    /*if (gameFadeTimer > 0.0f)
    {
        DxLib::SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * gameFadeTimer));
        DxPlus::Primitive2D::DrawRect({ 0,0 }, { DxPlus::CLIENT_WIDTH, DxPlus::CLIENT_HEIGHT }, DxLib::GetColor(0, 0, 0));
        DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }*/
}

void Game_End()
{

}