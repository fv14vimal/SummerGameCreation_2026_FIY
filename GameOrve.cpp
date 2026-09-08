#include "GameOrve.h"
#include "DxPlus/DxPlus.h"
#include "WinMain.h"
extern int nextScene;
int gameOverBGM;
int gameOverID;
void GameOver_Init()
{
    gameOverID = DxPlus::Sprite::Load(
        L"./Data/Images/GameOrve.png");

    if (gameOverID == -1)
    {
        DxPlus::Utils::FatalError(
            L"failed to load sprite : ./Data/Images/GameOrve.png");
    }

    gameOverBGM = DxLib::LoadSoundMem(
        L"./Data/Sounds/game_over.mp3");

    if (gameOverBGM == -1)
    {
        DxPlus::Utils::FatalError(
            L"failed to load sound : ./Data/Sounds/game_over.mp3");
    }

    DxLib::PlaySoundMem(gameOverBGM, DX_PLAYTYPE_LOOP);
}
void GameOver_Update()
{
	if (DxLib::CheckHitKey(KEY_INPUT_SPACE))
	{
		nextScene = SceneTitle;
	}
}
void GameOver_Render()
{
	if (gameOverID != -1)
	{
		DxPlus::Sprite::Draw(gameOverID);
	}
}
void GameOver_End()
{
    DxLib::StopSoundMem(gameOverBGM);

    if (gameOverBGM != -1)
    {
        DxLib::DeleteSoundMem(gameOverBGM);
        gameOverBGM = -1;
    }

    if (gameOverID != -1)
    {
        DxPlus::Sprite::Delete(gameOverID);
        gameOverID = -1;
    }
}