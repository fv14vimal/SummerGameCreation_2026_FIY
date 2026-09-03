#include "Title.h"
#include "DxPlus/DxPlus.h"
#include "WinMain.h"

//----------------------------------------------------------------------
// 定数
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// 変数
//----------------------------------------------------------------------
// フレームカウント用変数
int frameCount;
extern int nextScene;
int titleState;
float titleFadeTimer;

int titleID;

//----------------------------------------------------------------------
// 初期設定
//----------------------------------------------------------------------
void Title_Init()
{
    DxLib::SetBackgroundColor(255, 128, 0);
    titleID = DxPlus::Sprite::Load(L"./Data/Images/title.png");
    if (titleID == -1)
    {
        DxPlus::Utils::FatalError(L"failed to load sprite : ./Data/Images/title.png");
    }

    Title_Reset();
}

//----------------------------------------------------------------------
// リセット
//----------------------------------------------------------------------
void Title_Reset()
{
    frameCount = 0;
    titleState = 0;
    titleFadeTimer = 1.0f;
}

//----------------------------------------------------------------------
// 更新処理
//----------------------------------------------------------------------
void Title_Update()
{
    switch (titleState)
    {
        case 0: // フェードイン中
        {
            titleFadeTimer -= 1 / 60.0f;
            if (titleFadeTimer < 0.0f)
            {
                titleFadeTimer = 0.0f;
                titleState++;
            }
            break;
        }

        case 1: // 通常時
        {
            // EnterキーでGameシーンへ
            int input = DxPlus::Input::GetButtonDown(DxPlus::Input::PLAYER1);
            if (input & DxPlus::Input::BUTTON_START)
            {
                titleState++;
            }

            break;
        }

        case 2: // フェードアウト中
        {
            titleFadeTimer += 1 / 60.0f;
            if (titleFadeTimer > 1.0f)
            {
                titleFadeTimer = 1.0f;
                nextScene = SceneGame;
            }
            break;
        }
    }

    frameCount++;
}

//----------------------------------------------------------------------
// 描画処理
//----------------------------------------------------------------------
void Title_Render()
{
    if (titleID != -1)
    {
        DxPlus::Sprite::Draw(titleID);
    }

    // Push Enterの点滅
    if (frameCount & 0x20)
    {
        DxPlus::Text::DrawString(L"Push Enter", 
            { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.75f }, 
            DxLib::GetColor(255,255,0), DxPlus::Text::TextAlign::MIDDLE_CENTER, 
            { 2.0f, 2.0f });
    }

    // フェードイン / フェードアウト用
    if (titleFadeTimer > 0.0f)
    {
        DxLib::SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * titleFadeTimer));
        DxPlus::Primitive2D::DrawRect({ 0,0 }, 
            { DxPlus::CLIENT_WIDTH, DxPlus::CLIENT_HEIGHT}, DxLib::GetColor(0, 0, 0));
        DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

//----------------------------------------------------------------------
// 終了処理
//----------------------------------------------------------------------
void Title_End()
{
    if (titleID != -1)
    {
        DxPlus::Sprite::Delete(titleID);
        titleID = -1;
    }
}
