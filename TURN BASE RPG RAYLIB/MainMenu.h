#ifndef MAINMENU_H
#define MAINMENU_H

#include "raylib.h"

struct Button {
    Rectangle rect;
    const char* text;
    bool hovered;

    Button() : rect{ 0, 0, 0, 0 }, text(""), hovered(false) {}
    Button(float x, float y, float w, float h, const char* t);

    void Draw() const;
    bool IsMouseOver(Vector2 mousePos) const;
};

class MainMenu {
public:
    MainMenu(int screenWidth, int screenHeight);

    // Tampilkan menu utama, return true jika Start ditekan, false jika exit
    bool Show();

private:
    int screenWidth;
    int screenHeight;

    Button btnStart;
    Button btnCredit;
    Button btnExit;

    void ShowCredits() const;
};

#endif // MAINMENU_H
