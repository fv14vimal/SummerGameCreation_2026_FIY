#include "GameClear.h"
#include "DxPlus/DxPLus.h"
#include "WinMain.h"

extern int nextScene;

int gameClearID;

void GameClear_Init()
{
	gameClearID = DxPlus::Sprite::Load(L"./Data/Images/GameClear.png");
	if (gameClearID == -1)
	{
		DxPlus::Utils::FatalError(L"failed to load sprite : ./Data/Images/GameClear.png");
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
		DxPlus::Sprite::Draw(gameClearID);
	}
}

void GameClear_End()
{
	if (gameClearID != -1)
	{
		DxPlus::Sprite::Delete(gameClearID);
		gameClearID = -1;
	}
}