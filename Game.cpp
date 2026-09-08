#include "Game.h"
#include "DxPlus/DxPlus.h"
#include "WinMain.h"
#include "Entity2D.h"
#include <cmath>
#include "GameOrve.h"
#include "GameClear.h"
#include "Skill.h"

constexpr float MOVE_SPEED = 3.0f;
constexpr float PLAYER_SCALE_MAG = 0.05f;
constexpr float WALL_HALF_THICKNESS = 1.5f;
constexpr float FLOOR_SNAP_TOLERANCE = 5.0f;
constexpr DxPlus::Vec2 ARROW_POSITION = { 1180.0f, 60.0f };
constexpr float GRAVITY = 0.5f;          // 重力加速度
constexpr float MAX_FALL_SPEED = 8.0f;    // 落下速度の上限

constexpr float END_ZONE_X = 1150.0f;
constexpr float GATE_TARGET_WIDTH = 130.0f;
constexpr float SPIDER_TARGET_WIDTH = 80.0f;
constexpr float SPIDER_MOVE_SPEED = 1.2f; // 上下移動の速さ（少しゆっくりにして通りやすくした）
constexpr float SPIDER_HITBOX_SCALE = 0.55f; // 見た目より少し小さめの当たり判定にする
constexpr float SWITCH_TARGET_WIDTH = 55.0f;
constexpr DxPlus::Vec2 SWITCH_POSITION = { 1150.0f, 410.0f }; // Spring Floor(y=430)の右端寄りに設置

constexpr DxPlus::Vec2 GATE_POSITIONS[] =
{
    { 52.0f, 32.0f },
    { 454.0f, 40.0f },
    { 916.0f, 40.0f }
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

// スイッチで開閉する床。isOpen が false の間は完全に乗れない（すり抜ける）
struct GatedFloor
{
    Line line;
    bool isOpen;
};

// Spring Floor(y=430)の左端（section2寄り）は最初は閉じていて、
// 右端に置いたスイッチを押すと開通する
static GatedFloor springFloorGate = { {{854.0f, 430.0f}, {920.0f, 430.0f}}, false };

// クモの敵キャラ。指定した範囲を上下に往復する
struct Spider
{
    DxPlus::Vec2 position;
    float topY;
    float bottomY;
    float direction; // 1.0f = 下へ, -1.0f = 上へ
};

// クモの初期配置（X座標, 上端Y, 下端Y）
// 各クモは足場と足場の間の「隙間の中央部分」だけを往復させ、
// 足場のすぐ上下には安全な余白を残すことで、必ず通り抜けられるようにしている
constexpr struct { float x; float topY; float bottomY; } SPIDER_SPAWNS[] =
{
    // Section 1: between the y=240 floor and the y=440 floor (overlap x: 150-300)
    { 225.0f, 270.0f, 410.0f },

    // Section 2, gap 1: between the y=140 floor and the y=290 floor (overlap x: 550-700)
    { 615.0f, 190.0f, 240.0f },
    // Section 2, gap 3: between the y=400 floor and the y=550 floor (overlap x: 550-600)
    { 575.0f, 430.0f, 480.0f },

    // Section 3: guarding the approach to the switch, between the y=300 platform and the y=430 floor
    { 950.0f, 330.0f, 400.0f },
    { 1010.0f, 330.0f, 400.0f },
};
constexpr int SPIDER_COUNT = sizeof(SPIDER_SPAWNS) / sizeof(SPIDER_SPAWNS[0]);
static Spider spiders[SPIDER_COUNT];

constexpr Line sideLines[] =
{
    // Section 1
    {{0, 240}, {300, 240}},
    {{150, 440}, {427, 440}},
    // Section 2
    {{427, 140}, {700, 140}},
    {{550, 290}, {854, 290}},
    {{427, 400}, {600, 400}},
    {{550, 550}, {854, 550}},
    // Section 3
    {{854, 190}, {1100, 190}}, // Upper Platform
    {{934, 300}, {1070, 300}}, // Switch Platform (normal, always open)
    {{920, 430}, {1165, 430}}, // Spring Floor: always-open right portion (left portion is gated)
    {{1165, 430}, {1200, 430}},
    {{1024, 550}, {1165, 550}}
};
constexpr int SIDE_LINE_COUNT = sizeof(sideLines) / sizeof(sideLines[0]);

constexpr float WALL_X[] = { 427.0f, 854.0f };
constexpr int WALL_COUNT = sizeof(WALL_X) / sizeof(WALL_X[0]);
static WallRect wallRects[WALL_COUNT];

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
int spiderID;
<<<<<<< Updated upstream
int cheeseID;
=======
int switchOffID;
int switchOnID;
>>>>>>> Stashed changes

Entity2D player;
bool wasPressed = false;
bool isMovingRight = false;
bool reachedEnd = false;
bool isGameOver = false;

static DxPlus::Vec2 playerCenterPx = { 0.0f, 0.0f };
static DxPlus::Vec2 arrowCenterPx = { 0.0f, 0.0f };
static DxPlus::Vec2 gateCenterPx = { 0.0f, 0.0f };

static float playerRadius = 0.0f;
static float gateScale = 1.0f;

static DxPlus::Vec2 spiderCenterPx = { 0.0f, 0.0f };
static float spiderRadius = 0.0f;
static float spiderScale = 1.0f;

static DxPlus::Vec2 switchCenterPx = { 0.0f, 0.0f };
static float switchRadius = 0.0f;
static float switchScale = 1.0f;

void Game_Init()
{
    DxLib::SetBackgroundColor(0, 0, 0);
    playerID = DxPlus::Sprite::Load(L"./Data/Images/mouse.png");
    arrowID = DxPlus::Sprite::Load(L"./Data/Images/arrow.png");
    backID = DxPlus::Sprite::Load(L"./Data/Images/background.png");
    gateID = DxPlus::Sprite::Load(L"./Data/Images/gate.png");
    spiderID = DxPlus::Sprite::Load(L"./Data/Images/spider.png");
<<<<<<< Updated upstream
    cheeseID = DxPlus::Sprite::Load(L"./Data/Images/cheese.png");
=======
    switchOffID = DxPlus::Sprite::Load(L"./Data/Images/スイッチ.png");
    switchOnID = DxPlus::Sprite::Load(L"./Data/Images/スイッチ_押した状態_.png");
>>>>>>> Stashed changes

    if (gateID == -1) DxPlus::Utils::FatalError(L"Failed to load sprite: ./Data/Images/gate.png");
    if (spiderID == -1) DxPlus::Utils::FatalError(L"Failed to load sprite: ./Data/Images/spider.png");
    if (switchOffID == -1) DxPlus::Utils::FatalError(L"Failed to load sprite: ./Data/Images/スイッチ.png");
    if (switchOnID == -1) DxPlus::Utils::FatalError(L"Failed to load sprite: ./Data/Images/スイッチ_押した状態_.png");

    int imgW = 0, imgH = 0;
    DxLib::GetGraphSize(playerID, &imgW, &imgH);
    playerCenterPx = { imgW * 0.5f, imgH * 0.5f };
    playerRadius = imgW * PLAYER_SCALE_MAG * 0.5f;

    int arrowW = 0, arrowH = 0;
    DxLib::GetGraphSize(arrowID, &arrowW, &arrowH);
    arrowCenterPx = { arrowW * 0.5f, arrowH * 0.5f };

    int gateW = 0, gateH = 0;
    DxLib::GetGraphSize(gateID, &gateW, &gateH);
    gateCenterPx = { gateW * 0.5f, gateH * 0.5f };
    gateScale = (gateW > 0) ? (GATE_TARGET_WIDTH / gateW) : 1.0f;

    int spiderW = 0, spiderH = 0;
    DxLib::GetGraphSize(spiderID, &spiderW, &spiderH);
    spiderCenterPx = { spiderW * 0.5f, spiderH * 0.5f };
    spiderScale = (spiderW > 0) ? (SPIDER_TARGET_WIDTH / spiderW) : 1.0f;
    spiderRadius = spiderW * spiderScale * 0.5f * SPIDER_HITBOX_SCALE;

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
    isMovingRight = false;
    wasPressed = false;
    reachedEnd = false;
    isGameOver = false;
    springFloorGate.isOpen = false;

    for (int i = 0; i < SPIDER_COUNT; ++i)
    {
        spiders[i].position = { SPIDER_SPAWNS[i].x, SPIDER_SPAWNS[i].topY };
        spiders[i].topY = SPIDER_SPAWNS[i].topY;
        spiders[i].bottomY = SPIDER_SPAWNS[i].bottomY;
        spiders[i].direction = 1.0f;
    }

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

    // スイッチが押されるまでは springFloorGate には乗れない
    if (springFloorGate.isOpen)
    {
        const Line& line = springFloorGate.line;
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

void CheckSwitchActivation()
{
    if (springFloorGate.isOpen) return;

    float dx = player.position.x - SWITCH_POSITION.x;
    float dy = player.position.y - SWITCH_POSITION.y;
    float radiusSum = playerRadius + switchRadius;

    if (dx * dx + dy * dy < radiusSum * radiusSum)
    {
        springFloorGate.isOpen = true;
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

void UpdateSpiders()
{
    for (int i = 0; i < SPIDER_COUNT; ++i)
    {
        Spider& spider = spiders[i];
        spider.position.y += SPIDER_MOVE_SPEED * spider.direction;

        if (spider.position.y >= spider.bottomY)
        {
            spider.position.y = spider.bottomY;
            spider.direction = -1.0f;
        }
        else if (spider.position.y <= spider.topY)
        {
            spider.position.y = spider.topY;
            spider.direction = 1.0f;
        }
    }
}

void CheckSpiderCollisions()
{
    if (isGameOver || reachedEnd) return;

    for (int i = 0; i < SPIDER_COUNT; ++i)
    {
        const Spider& spider = spiders[i];
        float dx = player.position.x - spider.position.x;
        float dy = player.position.y - spider.position.y;
        float radiusSum = playerRadius + spiderRadius;

        if (dx * dx + dy * dy < radiusSum * radiusSum)
        {
            isGameOver = true;
            gameState = 2; // Reuse the existing fade-out flow
            return;
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
            // NOTE: add "SceneGameOver" to your scene enum in WinMain.h once the GameOver scene exists
            nextScene = reachedEnd ? SceneGameClear : (isGameOver ? SceneGameOver : SceneTitle);
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

    // Apply Gravity
    player.velocity.y += GRAVITY;
    if (player.velocity.y > MAX_FALL_SPEED) player.velocity.y = MAX_FALL_SPEED;
    player.position.y += player.velocity.y;

    ResolveWallCollisions(prevX);
    ResolveFloorCollisions(prevY);
    CheckSwitchActivation();

    UpdateSpiders();
    CheckSpiderCollisions();

    // Screen Bounds Collision
    if (player.position.x < playerRadius) player.position.x = playerRadius;
    if (player.position.x > DxPlus::CLIENT_WIDTH - playerRadius) player.position.x = DxPlus::CLIENT_WIDTH - playerRadius;
    if (player.position.y < playerRadius)
    {
        player.position.y = playerRadius;
        player.velocity.y = 0.0f;
    }
    if (player.position.y > DxPlus::CLIENT_HEIGHT - playerRadius)
    {
        player.position.y = DxPlus::CLIENT_HEIGHT - playerRadius;
        player.velocity.y = 0.0f;
    }

    CheckEnding();
}

void Game_Render()
{
    constexpr DxPlus::Vec2 BG_SCALE = { 1.6f, 1.6f };
    constexpr DxPlus::Vec2 BG_CENTER = { 0.0f, 0.0f };
    DxPlus::Sprite::Draw(backID, { 0.0f, 0.0f }, BG_SCALE, BG_CENTER);

    // Section Dividers & Outer Boundaries
    DxPlus::Primitive2D::DrawLine({ 427, 0 }, { 427, 720 }, DxLib::GetColor(0, 0, 0), 3.0f);
    DxPlus::Primitive2D::DrawLine({ 854, 0 }, { 854, 720 }, DxLib::GetColor(0, 0, 0), 3.0f);
    DxPlus::Primitive2D::DrawLine({ 0, 0 }, { 0, 720 }, DxLib::GetColor(0, 0, 0), 3.0f);
    DxPlus::Primitive2D::DrawLine({ 1280, 0 }, { 1280, 720 }, DxLib::GetColor(0, 0, 0), 3.0f);
    DxPlus::Primitive2D::DrawLine({ 0, 0 }, { 1280, 0 }, DxLib::GetColor(0, 0, 0), 3.0f);
    DxPlus::Primitive2D::DrawLine({ 0, 720 }, { 1280, 720 }, DxLib::GetColor(0, 0, 0), 3.0f);

    // SIDE 1
    DxPlus::Primitive2D::DrawLine({ 0, 240 }, { 300, 240 }, DxLib::GetColor(194, 29, 17), 5.0f);
    DxPlus::Primitive2D::DrawLine({ 150, 440 }, { 427, 440 }, DxLib::GetColor(194, 29, 17), 5.0f);

    // SIDE 2
    DxPlus::Primitive2D::DrawLine({ 427, 140 }, { 700, 140 }, DxLib::GetColor(194, 29, 17), 5.0f);
    DxPlus::Primitive2D::DrawLine({ 550, 290 }, { 854, 290 }, DxLib::GetColor(194, 29, 17), 5.0f);
    DxPlus::Primitive2D::DrawLine({ 427, 400 }, { 600, 400 }, DxLib::GetColor(194, 29, 17), 5.0f);
    DxPlus::Primitive2D::DrawLine({ 550, 550 }, { 854, 550 }, DxLib::GetColor(194, 29, 17), 5.0f);

    // SIDE 3
    DxPlus::Primitive2D::DrawLine({ 854, 190 }, { 1100, 190 }, DxLib::GetColor(194, 29, 17), 5.0f);
    DxPlus::Primitive2D::DrawLine({ 934, 300 }, { 1070, 300 }, DxLib::GetColor(194, 29, 17), 5.0f); // Switch platform (normal, always open)
    if (springFloorGate.isOpen)
    {
        DxPlus::Primitive2D::DrawLine({ 854, 430 }, { 920, 430 }, DxLib::GetColor(194, 29, 17), 5.0f); // Gated portion (opens after the switch is pressed)
    }
    DxPlus::Primitive2D::DrawLine({ 920, 430 }, { 1165, 430 }, DxLib::GetColor(194, 29, 17), 5.0f); // Always-open portion, before the gate
    DxPlus::Primitive2D::DrawLine({ 1165, 430 }, { 1200, 430 }, DxLib::GetColor(194, 29, 17), 5.0f);
    DxPlus::Primitive2D::DrawLine({ 1024, 550 }, { 1165, 550 }, DxLib::GetColor(194, 29, 17), 5.0f);

    // Render Gate Sprites
    for (int i = 0; i < GATE_COUNT; ++i)
    {
        DxPlus::Sprite::Draw(gateID, GATE_POSITIONS[i], { gateScale, gateScale }, gateCenterPx);
    }

    DxPlus::Sprite::Draw(cheeseID, { 1250.0f, 700.0f }, { gateScale, gateScale }, gateCenterPx);

    // Render Spiders
    for (int i = 0; i < SPIDER_COUNT; ++i)
    {
        DxPlus::Sprite::Draw(spiderID, spiders[i].position, { spiderScale, spiderScale }, spiderCenterPx);
    }

    // Render Switch (off/on state)
    int currentSwitchID = springFloorGate.isOpen ? switchOnID : switchOffID;
    DxPlus::Sprite::Draw(currentSwitchID, SWITCH_POSITION, { switchScale, switchScale }, switchCenterPx);

    // Render Player
    if (player.isActive)
    {
        float scaleX = isMovingRight ? -PLAYER_SCALE_MAG : PLAYER_SCALE_MAG;
        DxPlus::Vec2 playerScale = { scaleX, PLAYER_SCALE_MAG };
        DxPlus::Sprite::Draw(player.spriteID, player.position, playerScale, playerCenterPx);

        float arrowRotation = isMovingRight ? DxPlus::Deg2Rad * 180.0f : 0.0f;
        DxPlus::Sprite::Draw(arrowID, ARROW_POSITION, { 0.05f, 0.05f }, arrowCenterPx, arrowRotation);
    }
}

void Game_End()
{
}