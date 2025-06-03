#include "raylib.h"
#include "MainMenu.h"
#include "Game.h"


std::string EnterPlayerName() {
    std::string name = "";
    bool done = false;

    while (!done && !WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Enter your name:", 100, 100, 24, DARKGREEN);
        DrawRectangle(100, 140, 400, 40, LIGHTGRAY);
        DrawText(name.c_str(), 110, 150, 20, BLACK);
        DrawText("Press ENTER to continue", 100, 200, 20, GRAY);
        EndDrawing();

        int key = GetCharPressed();
        if (key >= 32 && key <= 125 && name.length() < 16) {
            name += static_cast<char>(key);
        }

        if (IsKeyPressed(KEY_BACKSPACE) && !name.empty()) {
            name.pop_back();
        }

        if (IsKeyPressed(KEY_ENTER) && !name.empty()) {
            done = true;
        }
    }

    return name;
}

void ShowWelcomeMessage(const std::string& name) {
    const float fadeDuration = 1.0f;     // 1 detik fade in & out
    const float holdDuration = 1.5f;     // waktu tampil penuh
    const float totalDuration = fadeDuration * 2 + holdDuration;

    float timer = 0.0f;

    while (!WindowShouldClose() && timer < totalDuration) {
        float alpha = 1.0f;

        if (timer < fadeDuration) {
            alpha = timer / fadeDuration;  // Fade-in
        }
        else if (timer > fadeDuration + holdDuration) {
            alpha = 1.0f - (timer - fadeDuration - holdDuration) / fadeDuration; // Fade-out
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        Color fadeColor = Fade(DARKGREEN, alpha);
        std::string msg = "Welcome back, " + name + "!";
        int textWidth = MeasureText(msg.c_str(), 30);
        DrawText(msg.c_str(), (800 - textWidth) / 2, 200, 30, fadeColor);

        EndDrawing();
        timer += GetFrameTime();
    }
}



int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "2D Turn-Based RPG");

    MainMenu menu(screenWidth, screenHeight);
    Game* game = new Game(screenWidth, screenHeight);

    bool running = true;

    while (!WindowShouldClose() && running) {
        switch (game->state) {
        case GameState::MainMenu: {
            bool startGame = menu.Show();
            if (!startGame) {
                running = false;
            }
            else {
                game->Unload();               // unload resources before reset
                delete game;
                game = new Game(screenWidth, screenHeight);
                game->state = GameState::Arena;
                if (game->GetPlayerName().empty()) {
                    game->SetPlayerName(EnterPlayerName());
                    game->SaveGame();
                }
                else {
                    ShowWelcomeMessage(game->GetPlayerName());
                }
            }
            break;
        }
        case GameState::Arena:
            game->ShowTownSquare();
            break;
        case GameState::Shop:
            game->ShowShop();
            break;
        case GameState::Battle:
            game->StartBattle();
            break;
        case GameState::Exit:
            running = false;
            break;
        }
    }

    game->Unload();  // ✅ pastikan resource dibersihkan
    delete game;
    CloseWindow();
    return 0;
}
