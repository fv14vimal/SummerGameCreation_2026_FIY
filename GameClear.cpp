#include "GameClear.h"
#include "DxPlus/DxPlus.h"
#include "WinMain.h"

extern int nextScene;

void GameClear_Init()
{
	DxLib::SetBackgroundColor(35, 88, 50);
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
	DxPlus::Text::DrawString(L"GAME CLEAR!",
		{ DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.42f },
		DxLib::GetColor(255, 236, 89), DxPlus::Text::TextAlign::MIDDLE_CENTER,
		{ 2.5f, 2.5f });
	DxPlus::Text::DrawString(L"Push SPACE to return to the title",
		{ DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.58f },
		DxLib::GetColor(255, 255, 255), DxPlus::Text::TextAlign::MIDDLE_CENTER,
		{ 1.2f, 1.2f });
}

void GameClear_End()
{
}
