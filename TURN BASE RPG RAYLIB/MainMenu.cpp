#include "MainMenu.h"

#include <iostream>

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

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (btnStart.hovered) {
                return true;  // Mulai game
            }
            else if (btnCredit.hovered) {
                BeginDrawing();
                ShowCredits();
                BeginDrawing();
            }
            else if (btnExit.hovered) {
                CloseWindow();
                return false;  // Keluar game
            }
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
