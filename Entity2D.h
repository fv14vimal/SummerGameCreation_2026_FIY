#pragma once

#include "DxPlus/DxPlus.h"

// TODO:01 Entity2D構造体の作成
/*
課題）
    下記に Entity2D 構造体を作成しましょう
    [メンバ変数]
        DxPlus::Vec2 position;  // 位置
        DxPlus::Vec2 velocity;  // 速度
        int spriteID;           // 描画すべき画像ID（のコピー）
        bool isActive;          // 存在フラグ

        // アニメーション制御（プレイヤー用）
        int animStepIndex;       // 現在のアニメーションステップ
        int animTimer;           // そのステップで経過したフレーム数
        int spriteIndex;         // 分割画像の何コマ目か

解説）
    背景やプレイヤー、土管など、画面上に表示・移動する全てのオブジェクトを
    この構造体を使って表現します。
    アニメーション用変数は、主にTODO:12番以降で使用します。
*/
// TODO_01


