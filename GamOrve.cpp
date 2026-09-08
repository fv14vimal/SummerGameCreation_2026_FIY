#include "GameOrve.h"
#include "DxPlus/DxPlus.h"
#include "WinMain.h"
extern int nextScene;
int gameOverID;
void GameOver_Init()
{
	gameOverID = DxPlus::Sprite::Load(L"./Data/Images/GameOver.png");
	if (gameOverID == -1)
	{
		DxPlus::Utils::FatalError(L"failed to load sprite : ./Data/Images/GameOver.png");
	}
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
	if (gameOverID != -1)
	{
		DxPlus::Sprite::Delete(gameOverID);
		gameOverID = -1;
	}
}