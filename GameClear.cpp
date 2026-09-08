#include "GameClear.h"
#include "DxPlus/DxPlus.h"
#include "WinMain.h"
extern int nextScene;
int gameClearID;
int gameClearBGM;
static DxPlus::Vec2 gameClearScale = { 1.0f, 1.0f };
void GameClear_Init()
{
    DxLib::SetBackgroundColor(35, 88, 50);

    gameClearID = DxPlus::Sprite::Load(L"./Data/Images/GameClear.png");

    if (gameClearID == -1)
    {
        DxPlus::Utils::FatalError(
            L"failed to load sprite : ./Data/Images/GameClear.png");
    }

    gameClearBGM = DxLib::LoadSoundMem(
        L"./Data/Sounds/game_clear.mp3");

    if (gameClearBGM == -1)
    {
        DxPlus::Utils::FatalError(
            L"failed to load sound : ./Data/Sounds/game_clear.mp3");
    }

    DxLib::PlaySoundMem(gameClearBGM, DX_PLAYTYPE_LOOP);

    int imgW = 0, imgH = 0;
    DxLib::GetGraphSize(gameClearID, &imgW, &imgH);

    if (imgW > 0 && imgH > 0)
    {
        gameClearScale.x =
            static_cast<float>(DxPlus::CLIENT_WIDTH) / imgW;

        gameClearScale.y =
            static_cast<float>(DxPlus::CLIENT_HEIGHT) / imgH;
    }
}
void GameClear_Update()
{
	if (DxLib::CheckHitKey(KEY_INPUT_SPACE))
	{
		nextScene = SceneTitle;
	}
}
void GameClear_Render()
{
	if (gameClearID != -1)
	{
		DxPlus::Sprite::Draw(gameClearID, { 0.0f, 0.0f }, gameClearScale, { 0.0f, 0.0f });
	}

	
	DxPlus::Text::DrawString(L"Push SPACE to return to the title",
		{ DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.58f },
		DxLib::GetColor(255, 255, 255), DxPlus::Text::TextAlign::MIDDLE_CENTER,
		{ 1.2f, 1.2f });
}
void GameClear_End()
{
    DxLib::StopSoundMem(gameClearBGM);

    if (gameClearBGM != -1)
    {
        DxLib::DeleteSoundMem(gameClearBGM);
        gameClearBGM = -1;
    }

    if (gameClearID != -1)
    {
        DxPlus::Sprite::Delete(gameClearID);
        gameClearID = -1;
    }
}