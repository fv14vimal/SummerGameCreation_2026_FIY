// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#pragma once
#include "Vector2.h"

namespace DxPlus
{
    struct AnimationFrame {
        int imageId;    // 表示する画像ID
        int duration;   // そのフレームが表示される時間（フレーム数）
    };

    class Animation {
    public:
        void Update(AnimationFrame* animation, bool loop = true);
        void Draw(const DxPlus::Vec2& position, const DxPlus::Vec2& scale,
            const DxPlus::Vec2& center, float rotationZ, int color);
    private:
        AnimationFrame* currentAnimation;
        int frameCounter;
        int currentFrame;
    };
}
