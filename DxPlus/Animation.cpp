// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#include "Animation.h"
#include "Sprite.h"

namespace DxPlus
{
    void Animation::Update(AnimationFrame* animation, bool loop)
    {
        // nullptr が渡された場合、現在のアニメーションを継続
        if (animation == nullptr)
        {
            animation = currentAnimation;
        }

        // 別のアニメーション開始
        if (currentAnimation != animation)
        {
            currentAnimation = animation;
            currentFrame = 0;
            frameCounter = 0;
        }
        // 同じアニメーションが進む
        else
        {
            if (currentAnimation[currentFrame].duration < 0) { return; }

            frameCounter++;
            if (frameCounter >= currentAnimation[currentFrame].duration)
            {
                if (currentAnimation[currentFrame + 1].duration < 0)
                {
                    // 終了フラグ
                    if (loop)
                    {
                        currentFrame = 0;
                        frameCounter = 0;
                    }
                }
                else
                {
                    currentFrame++;
                    frameCounter = 0;
                }
            }
        }
    }

    void Animation::Draw(const DxPlus::Vec2& position, const DxPlus::Vec2& scale, const DxPlus::Vec2& center, float rotationZ, int color)
    {
        Sprite::Draw(currentAnimation[currentFrame].imageId,
            position, scale, center, rotationZ, color);
    }
}
