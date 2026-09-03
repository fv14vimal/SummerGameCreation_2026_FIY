#include "DxPlus/DxPlus.h"
// WinMain.hとTitle.hをインクルードする
#include "WinMain.h"
#include "Title.h"
#include "Game.h"

//----------------------------------------------------------------------
// 変数
//----------------------------------------------------------------------
// シーン遷移用の変数を宣言する
int currentScene = SceneNone;
int nextScene = SceneTitle;

//----------------------------------------------------------------------
// プロトタイプ宣言
//----------------------------------------------------------------------
static LRESULT CALLBACK CustomWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//----------------------------------------------------------------------
// wWinMain関数
//----------------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
    // DxPlus(DXライブラリ）の初期化
    constexpr bool WINDOWED = TRUE; // ウィンドウモード
    if (DxPlus::Initialize(DxPlus::CLIENT_WIDTH, DxPlus::CLIENT_HEIGHT, WINDOWED) == -1)
    {
        return -1;  // 初期化失敗
    }

    // ウィンドウプロシージャを設定
    DxLib::SetHookWinProc(CustomWinProc);

    // ゲームループ
    while (DxPlus::GameLoop())
    {
        // シーンが切り替わるなら
        if (nextScene != SceneNone)
        {
            // 今のシーンの終了関数を呼ぶ
            switch (currentScene)
            {
            case SceneTitle:
                Title_End();
                break;

            case SceneGame:
                Game_End();
                break;
            }

            // 次のシーンの初期設定関数を呼ぶ
            switch (nextScene)
            {
            case SceneTitle:
                Title_Init();
                break;

            case SceneGame:
                Game_Init();
                break;
            }

            // シーンの切り替えを反映させ、次のシーン指定をクリアしておく
            currentScene = nextScene;
            nextScene = SceneNone;
        }

        // 入力の状態を更新する
        DxPlus::Input::Update();

        // 今のシーンの更新処理と描画処理を呼ぶ
        switch (currentScene)
        {
        case SceneTitle:
            Title_Update();
            Title_Render();
            break;

        case SceneGame:
            Game_Update();
            Game_Render();
            break;
        }

        // デバッグ情報を描画する
        DxPlus::Debug::Draw();
        //SetWindowText(L"2dgp1_10 完成サンプル");

        // 画面をクリアする
        DxLib::ScreenFlip();
        DxLib::ClearDrawScreen();
    }

    // 終了処理関数を呼ぶ
    switch (currentScene)
    {
    case SceneTitle:
        Title_End();
        break;

    case SceneGame:
        Game_End();
        break;
    }

    // DxPlusの終了処理
    DxPlus::ShutDown();

    // 戻り値は0でよい
    return 0;
}

//----------------------------------------------------------------------
// ウィンドウプロシージャ
//----------------------------------------------------------------------
LRESULT CALLBACK CustomWinProc(HWND, UINT msg, WPARAM wParam, LPARAM)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
        }
        break;
    }

    return 0;   // ここはreturn 0;でOK
}
