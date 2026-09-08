#include "Skill.h"
#include "DxPlus/DxPlus.h"
#include "WinMain.h"

extern int nextScene;
int SkillID;

void Skill_Init()
{
	SkillID = DxPlus::Sprite::Load(L"./Data/Images/image.png");
	if (SkillID == -1)
	{
		DxPlus::Utils::FatalError(L"failed to load sprite : ./Data/Images/image.png");
	}
}

void Skill_Update()
{
	if (DxLib::CheckHitKey(KEY_INPUT_SPACE))
	{
		nextScene = SceneGame;
	}
}

void Skill_Render()
{
	if (SkillID != -1)
	{
		DxPlus::Sprite::Draw(SkillID);
	}
}

void Skill_End()
{
	if (SkillID != -1)
	{
		DxPlus::Sprite::Delete(SkillID);
		SkillID = -1;
	}
}