#include "MainMenu.h"
#include "raylib.h"
#include <iostream>


void ShowLoadingScreen(const char* message, int totalSteps = 10) {
    double loadingStart = GetTime();

    for (int step = 0; step <= totalSteps && !WindowShouldClose(); ++step) {
        BeginDrawing();
        ClearBackground(WHITE); // Set background to white

        // Draw loading message
        int fontSize = 30;
        int textWidth = MeasureText(message, fontSize);
        DrawText(message, GetScreenWidth() / 2 - textWidth / 2, GetScreenHeight() / 2 - 80, fontSize, BLACK);

        // Draw animated dots
        int dotCount = (int)(GetTime() * 2) % 4; // cycles 0-3
        std::string dots(dotCount, '.');
        int dotsWidth = MeasureText(dots.c_str(), fontSize);
        DrawText(dots.c_str(), GetScreenWidth() / 2 + textWidth / 2 + 10, GetScreenHeight() / 2 - 80, fontSize, BLACK);

        // Draw progress bar background
        int barWidth = 400;
        int barHeight = 30;
        int barX = GetScreenWidth() / 2 - barWidth / 2;
        int barY = GetScreenHeight() / 2;
        DrawRectangle(barX, barY, barWidth, barHeight, DARKGRAY);

        // Draw progress bar fill
        float progress = (float)step / totalSteps;
        DrawRectangle(barX, barY, (int)(barWidth * progress), barHeight, SKYBLUE);

        // Draw progress percent
        char percentText[16];
        snprintf(percentText, sizeof(percentText), "%d%%", (int)(progress * 100));
        int percentWidth = MeasureText(percentText, 20);
        DrawText(percentText, GetScreenWidth() / 2 - percentWidth / 2, barY + barHeight + 10, 20, BLACK);

        EndDrawing();
    }
}

// Implementasi Button
Button::Button(float x, float y, float w, float h, const char* t)
    : rect{ x, y, w, h }, text(t), hovered(false) {
}

void Button::Draw() const {
    Color bgColor = hovered ? DARKBLUE : BLUE;
    DrawRectangleRec(rect, bgColor);

    int fontSize = 20;
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, rect.x + (rect.width - textWidth) / 2, rect.y + (rect.height - fontSize) / 2, fontSize, WHITE);
}

bool Button::IsMouseOver(Vector2 mousePos) const {
    return CheckCollisionPointRec(mousePos, rect);
}

// Implementasi MainMenu

MainMenu::MainMenu(int screenW, int screenH)
    : screenWidth(screenW), screenHeight(screenH)
{
    float btnWidth = 200;
    float btnHeight = 50;
    float spacing = 20;

    float totalHeight = btnHeight * 3 + spacing * 2;
    float startY = (screenHeight - totalHeight) / 2;
    float centerX = screenWidth / 2 - btnWidth / 2;

    btnStart = Button(centerX, startY, btnWidth, btnHeight, "Start");
    btnCredit = Button(centerX, startY + btnHeight + spacing, btnWidth, btnHeight, "Credit");
    btnExit = Button(centerX, startY + (btnHeight + spacing) * 2, btnWidth, btnHeight, "Exit");
}



void MainMenu::ShowCredits() const {
    // Loop tunggu input dengan drawing aktif
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("Created by aku", screenWidth / 2 - 100, screenHeight / 2 - 20, 20, WHITE);
        DrawText("Press any key or click to return", screenWidth / 2 - 160, screenHeight / 2 + 20, 20, WHITE);

        EndDrawing();

        // Jika user tekan tombol apapun atau klik mouse, keluar dari credits
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            break;
        }
    }
}


bool MainMenu::Show() {
    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();

        btnStart.hovered = btnStart.IsMouseOver(mousePos);
        btnCredit.hovered = btnCredit.IsMouseOver(mousePos);
        btnExit.hovered = btnExit.IsMouseOver(mousePos);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && btnStart.hovered) {
            ShowLoadingScreen("Loading...", 1000); // Show for 1 second
            return true; // Indicate start was pressed
        }
        else if (btnCredit.hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            ShowCredits();
        }
        else if (btnExit.hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            CloseWindow();
            return false;  // Exit game
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Turn-Based RPG", screenWidth / 2 - MeasureText("Turn-Based RPG", 40) / 2, 50, 40, DARKBLUE);

        btnStart.Draw();
        btnCredit.Draw();
        btnExit.Draw();

        EndDrawing();
    }
    return false;
}

