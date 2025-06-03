#include "raylib.h"
#include "Game.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include "NotificationObserver.h"
#include "PlayerCommands.h"
#include <algorithm>
#include "ArcherFactory.h"
#include "WarriorFactory.h"
#include "PaladinFactory.h"
#include "WitchFactory.h"
#include "Enemy.h"


#ifdef DARKRED
#undef DARKRED
#endif
#ifdef DARKMAGENTA
#undef DARKMAGENTA
#endif
#ifdef DARKGREEN
#undef DARKGREEN
#endif
#ifdef DARKPURPLE
#undef DARKPURPLE
#endif

#define DARKRED CLITERAL(Color){139,0,0,255}
#define DARKMAGENTA CLITERAL(Color){139,0,139,255}
#define DARKGREEN CLITERAL(Color){0,100,0,255}
#define DARKPURPLE CLITERAL(Color){75,0,130,255}
#define DARKPURPLE CLITERAL(Color){75,0,130,255}
#define DARKGOLD CLITERAL(Color){184, 134, 11, 255} // warna gold gelap (DarkGoldenrod)


int Game::GetRandom(int min, int max) const {
    return min + (rand() % (max - min + 1));
}

Game::Game(int screenW, int screenH)
    : screenWidth(screenW), screenHeight(screenH),
    running(true), state(GameState::MainMenu),
    playerCoins(0), selectedAction(0),
    skillCooldownTurns(0),
    showAttackEffect(false), attackEffectFrame(0),
    isPlayerTurn(true), isBlocking(false), skillOnCooldown(false)
{
    characterTexture = LoadTexture("assets/character.png");

    // Initialize available skills
    availableSkills = {
        { "Blazing Strike", "A powerful fire attack. (Unlocks Skill in battle)", 50, false },
        { "Frost Guard", "Reduces damage for 2 turns. (Unlocks Skill in battle)", 40, false },
        { "Thunder Dash", "Quick attack, always goes first. (Unlocks Skill in battle)", 60, false }
    };

    srand(static_cast<unsigned int>(time(nullptr)));
    InitPlayer();   // Set default values
    LoadGame();     // Overwrite with saved values if available
}



void Game::Unload() {
    UnloadTexture(characterTexture);
}

bool Game::IsRunning() const {
    return running;
}

void Game::AddObserver(NotificationObserver* observer) {
    observers.push_back(observer);
}
void Game::RemoveObserver(NotificationObserver* observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

void Game::ShowNotification(const std::string& msg) {
    std::cout << "[Notification] " << msg << std::endl;
    for (auto* obs : observers) {
        obs->OnNotify(msg);
    }
    // Add to battle log if in battle
    if (state == GameState::Battle) {
        battleLog.push_back(msg);
        if (battleLog.size() > BATTLE_LOG_MAX_LINES) {
            battleLog.erase(battleLog.begin()); // Remove the oldest line
        }
    }
}

void Game::SetPlayerName(const std::string& name) {
    player.name = name;
}
std::string Game::GetPlayerName() const {
    return player.name;
}
std::string Game::EnterPlayerName() {
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


void Game::InitPlayer() {
    player.name = "";
    player.maxHP = 100;
    player.currentHP = 100;
    player.attack = 15;
    player.defense = 5;
    player.level = 1;
    player.exp = 0;
    player.expToLevel = 100;
    playerCoins = 0;
    inventory.clear();
    inventory.push_back({ "Potion", "Restores 20 HP", 3 });
}


struct ShopItem {
    std::string name;
    std::string description;
    int price;
};

const ShopItem shopItems[] = {
    { "Potion", "Restores 20 HP", 10 },
    { "Hi-Potion", "Restores 50 HP", 25 },
    { "Elixir", "Fully restores HP", 50 },
    { "Antidote", "Cures poison", 15 },
    { "Attack Up", "Boosts attack for next battle", 30 },
    { "Defense Up", "Boosts defense for next battle", 30 },
    { "Revive", "Revives you with 50% HP if defeated", 60 },
    { "Speed Boots", "Increases speed for next battle", 35 },
    { "Magic Water", "Restores skill cooldown instantly", 40 }
};
const int shopItemCount = sizeof(shopItems) / sizeof(shopItems[0]);




void Game::InitEnemy() {
    enemyType = static_cast<EnemyType>(GetRandom(1, 2));
    enemyLevel = std::max(1, player.level - 1 + GetRandom(0, 2));

    EnemyFactory* factory = nullptr;

    switch (enemyType) {
    case EnemyType::Archer:
        factory = new ArcherFactory();
        break;
    case EnemyType::Warrior:
        factory = new WarriorFactory();
        break;
    case EnemyType::Paladin:
        factory = new PaladinFactory();
        break;
    case EnemyType::Witch:
        factory = new WitchFactory();
        break;
    }

    Enemy* generated = factory->CreateEnemy(enemyLevel);

    enemy.name = generated->GetName();
    enemy.maxHP = generated->GetMaxHP();
    enemy.currentHP = enemy.maxHP;
    enemy.attack = generated->GetAttack();
    enemy.defense = generated->GetDefense();
    enemy.level = enemyLevel;

    baseEnemyExp = 20 + (enemyLevel * 5);
    baseEnemyCoins = 5 + (enemyLevel * 2);

    delete generated;
    delete factory;
}


void Game::ShowTownSquare() {
    state = GameState::TownSquare;
    Rectangle colosseumBtn = { 20, 120, 300, 40 };
    Rectangle marketBtn = { 20, 180, 300, 40 };
    Rectangle tavernBtn = { 20, 240, 300, 40 };
    Rectangle trainingBtn = { 20, 300, 300, 40 };
    Rectangle exitBtn = { (float)(screenWidth - 220), (float)(screenHeight - 80), 200, 50 };

    while (state == GameState::TownSquare && !WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("[Aetherion - TOWN SQUARE]", 20, 20, 30, DARKBLUE);

        Vector2 mousePos = GetMousePosition();

        // Draw buttons
        Color colColor = CheckCollisionPointRec(mousePos, colosseumBtn) ? GRAY : LIGHTGRAY;
        DrawRectangleRec(colosseumBtn, colColor);
        DrawText("1. Colosseum (Battle Arena)", colosseumBtn.x + 10, colosseumBtn.y + 10, 20, BLACK);

        Color marketColor = CheckCollisionPointRec(mousePos, marketBtn) ? GRAY : LIGHTGRAY;
        DrawRectangleRec(marketBtn, marketColor);
        DrawText("2. Market (Shop)", marketBtn.x + 10, marketBtn.y + 10, 20, BLACK);

        Color tavernColor = CheckCollisionPointRec(mousePos, tavernBtn) ? GRAY : LIGHTGRAY;
        DrawRectangleRec(tavernBtn, tavernColor);
        DrawText("3. Tavern (Heal/Save)", tavernBtn.x + 10, tavernBtn.y + 10, 20, BLACK);

        Color trainColor = CheckCollisionPointRec(mousePos, trainingBtn) ? GRAY : LIGHTGRAY;
        DrawRectangleRec(trainingBtn, trainColor);
        DrawText("4. Training Ground (Coming Soon)", trainingBtn.x + 10, trainingBtn.y + 10, 20, DARKGRAY);

        Color exitColor = CheckCollisionPointRec(mousePos, exitBtn) ? GRAY : LIGHTGRAY;
        DrawRectangleRec(exitBtn, exitColor);
        DrawText("Exit to Main Menu", exitBtn.x + 20, exitBtn.y + 15, 24, BLACK);

        EndDrawing();

        // Mouse input
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mousePos, colosseumBtn)) {
                ShowColosseum();
            }
            else if (CheckCollisionPointRec(mousePos, marketBtn)) {
                ShowMarket();
            }
            else if (CheckCollisionPointRec(mousePos, tavernBtn)) {
                ShowTavern();
            }
            else if (CheckCollisionPointRec(mousePos, trainingBtn)) {
                ShowTrainingGround();
            }
            else if (CheckCollisionPointRec(mousePos, exitBtn)) {
                running = false;
                state = GameState::MainMenu;
                break;
            }
        }

        // Keyboard input
        if (IsKeyPressed(KEY_ONE)) ShowColosseum();
        else if (IsKeyPressed(KEY_TWO)) ShowMarket();
        else if (IsKeyPressed(KEY_THREE)) ShowTavern();
        else if (IsKeyPressed(KEY_FOUR)) ShowTrainingGround();
        else if (IsKeyPressed(KEY_ESCAPE)) {
            running = false;
            state = GameState::MainMenu;
            break;
        }
    }
}

void Game::ShowColosseum() {
    state = GameState::Colosseum;
    Rectangle quickBtn = { 20, 120, 300, 40 };
    Rectangle survivalBtn = { 20, 180, 300, 40 };
    Rectangle backBtn = { 20, 240, 300, 40 };

    while (state == GameState::Colosseum && !WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Colosseum (Battle Arena)", 20, 20, 30, DARKRED);

        Vector2 mousePos = GetMousePosition();

        Color quickColor = CheckCollisionPointRec(mousePos, quickBtn) ? GRAY : LIGHTGRAY;
        DrawRectangleRec(quickBtn, quickColor);
        DrawText("1. Quick Battle", quickBtn.x + 10, quickBtn.y + 10, 20, BLACK);

        Color survivalColor = CheckCollisionPointRec(mousePos, survivalBtn) ? GRAY : LIGHTGRAY;
        DrawRectangleRec(survivalBtn, survivalColor);
        DrawText("2. Survival Mode (Coming Soon)", survivalBtn.x + 10, survivalBtn.y + 10, 20, DARKGRAY);

        Color backColor = CheckCollisionPointRec(mousePos, backBtn) ? GRAY : LIGHTGRAY;
        DrawRectangleRec(backBtn, backColor);
        DrawText("3. Back to Town", backBtn.x + 10, backBtn.y + 10, 20, BLACK);

        EndDrawing();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mousePos, quickBtn)) {
                InitEnemy();
                skillCooldownTurns = 0;
                state = GameState::Battle;
                StartBattle();
                return;
            }
            else if (CheckCollisionPointRec(mousePos, survivalBtn)) {
                ShowNotification("Survival Mode coming soon!");
            }
            else if (CheckCollisionPointRec(mousePos, backBtn)) {
                ShowTownSquare();
                return;
            }
        }
        if (IsKeyPressed(KEY_ONE)) {
            InitEnemy();
            skillCooldownTurns = 0;
            state = GameState::Battle;
            StartBattle();
            return;
        }
        else if (IsKeyPressed(KEY_TWO)) {
            ShowNotification("Survival Mode coming soon!");
        }
        else if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_ESCAPE)) {
            ShowTownSquare();
            return;
        }
    }
}

void Game::ShowMarket() {
    state = GameState::Market;
    Rectangle shopBtn = { 20, 120, 300, 40 };
    Rectangle skillShopBtn = { 20, 180, 300, 40 };
    Rectangle backBtn = { 20, 240, 300, 40 };

    while (state == GameState::Market && !WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Market (Shop)", 20, 20, 30, DARKGOLD);

        Vector2 mousePos = GetMousePosition();

        // Shop button
        Color shopColor = CheckCollisionPointRec(mousePos, shopBtn) ? GRAY : LIGHTGRAY;
        DrawRectangleRec(shopBtn, shopColor);
        DrawText("1. Shop", shopBtn.x + 10, shopBtn.y + 10, 20, BLACK);

        // Skill Shop button
        Color skillColor = CheckCollisionPointRec(mousePos, skillShopBtn) ? GRAY : LIGHTGRAY;
        DrawRectangleRec(skillShopBtn, skillColor);
        DrawText("2. Arcane Skill Emporium", skillShopBtn.x + 10, skillShopBtn.y + 10, 20, DARKMAGENTA);

        // Back to Town button
        Color backColor = CheckCollisionPointRec(mousePos, backBtn) ? GRAY : LIGHTGRAY;
        DrawRectangleRec(backBtn, backColor);
        DrawText("3. Back to Town", backBtn.x + 10, backBtn.y + 10, 20, BLACK);

        EndDrawing();

        // Handle input
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mousePos, shopBtn)) {
                ShowShop();
            }
            else if (CheckCollisionPointRec(mousePos, skillShopBtn)) {
                ShowSkillShop();
            }
            else if (CheckCollisionPointRec(mousePos, backBtn)) {
                ShowTownSquare();
                return;
            }
        }

        if (IsKeyPressed(KEY_ONE)) {
            ShowShop();
        }
        else if (IsKeyPressed(KEY_TWO)) {
            ShowSkillShop();
        }
        else if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_ESCAPE)) {
            ShowTownSquare();
            return;
        }
    }
}

void Game::ShowTavern() {
    state = GameState::Tavern;
    Rectangle restBtn = { 20, 120, 300, 40 };
    Rectangle saveBtn = { 20, 180, 300, 40 };
    Rectangle loadBtn = { 20, 240, 300, 40 };
    Rectangle cottageBtn = { 20, 300, 300, 40 };
    Rectangle backBtn = { 20, 360, 300, 40 };

    while (state == GameState::Tavern && !WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Tavern", 20, 20, 30, DARKGREEN);

        Vector2 mousePos = GetMousePosition();

        auto drawButton = [&](Rectangle rect, const char* text) {
            Color btnColor = CheckCollisionPointRec(mousePos, rect) ? GRAY : LIGHTGRAY;
            DrawRectangleRec(rect, btnColor);
            DrawText(text, rect.x + 10, rect.y + 10, 20, BLACK);
            };

        drawButton(restBtn, "1. Rest (Heal HP) - 45 coins");
        drawButton(saveBtn, "2. Save Game");
        drawButton(loadBtn, "3. Load Game");
        drawButton(cottageBtn, "4. Cottage (View Stats & Inventory)");
        drawButton(backBtn, "5. Back to Town");

        EndDrawing();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mousePos, restBtn)) {
                if (playerCoins >= 45) {
                    playerCoins -= 45;
                    player.currentHP = player.maxHP;
                    ShowNotification("You are fully healed!");
                }
                else {
                    ShowNotification("Not enough coins to rest!");
                }
            }
            else if (CheckCollisionPointRec(mousePos, saveBtn)) {
                SaveGame();
                ShowNotification("Game Saved!");
            }
            else if (CheckCollisionPointRec(mousePos, loadBtn)) {
                LoadGame();
                ShowNotification("Game Loaded!");
            }
            else if (CheckCollisionPointRec(mousePos, cottageBtn)) {
                ShowPlayerStatsAndInventory(); // ⬅️ tampilkan menu stats
            }
            else if (CheckCollisionPointRec(mousePos, backBtn)) {
                ShowTownSquare();
                return;
            }
        }

        // Keyboard shortcuts
        if (IsKeyPressed(KEY_ONE)) {
            if (playerCoins >= 45) {
                playerCoins -= 45;
                player.currentHP = player.maxHP;
                ShowNotification("You are fully healed!");
            }
            else {
                ShowNotification("Not enough coins to rest!");
            }
        }
        else if (IsKeyPressed(KEY_TWO)) {
            SaveGame();
            ShowNotification("Game Saved!");
        }
        else if (IsKeyPressed(KEY_THREE)) {
            LoadGame();
            ShowNotification("Game Loaded!");
        }
        else if (IsKeyPressed(KEY_FOUR)) {
            ShowPlayerStatsAndInventory();
        }
        else if (IsKeyPressed(KEY_FIVE) || IsKeyPressed(KEY_ESCAPE)) {
            ShowTownSquare();
            return;
        }
    }
}

void Game::ShowPlayerStatsAndInventory() {
    enum class SubMenu { Stats, Inventory, Skills };
    SubMenu currentMenu = SubMenu::Stats;

    bool viewing = true;
    int selectedItemIndex = 0;
    int selectedSkillIndex = 0;
    Rectangle renameBtn = { 300, 300, 160, 30 };

    // Track which skill is selected for battle
    int equippedSkillIndex = -1;
    for (size_t i = 0; i < playerSkills.size(); ++i) {
        if (playerSkills[i].owned) {
            equippedSkillIndex = (int)i;
            break;
        }
    }

    while (viewing && !WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Cottage", 20, 20, 30, DARKGREEN);
        DrawText("[TAB] Switch Menu", 600, 20, 20, GRAY);

        Vector2 mousePos = GetMousePosition();

        Rectangle panel = { 40, 60, 700, 350 };
        DrawRectangleRec(panel, CLITERAL(Color){240, 240, 240, 255});
        DrawRectangleLinesEx(panel, 2, DARKGREEN);

        if (currentMenu == SubMenu::Stats) {
            DrawText("Player Stats", 60, 80, 25, DARKGREEN);

            // Character Image
            DrawTextureEx(characterTexture, Vector2{ 25, 110 }, 0.0f, 0.1f, WHITE);

            // Player Name
            DrawText(player.name.c_str(), 300, 100, 25, BLACK);

            // HP Text & Bar
            DrawText(("HP: " + std::to_string(player.currentHP) + "/" + std::to_string(player.maxHP)).c_str(), 300, 140, 20, BLACK);
            float hpPercent = (float)player.currentHP / player.maxHP;
            Rectangle hpBarBack = { 440, 140, 200, 20 };
            Rectangle hpBarFill = { 440, 140, 200 * hpPercent, 20 };
            DrawRectangleRec(hpBarBack, GRAY);
            DrawRectangleRec(hpBarFill, DARKRED);
            DrawRectangleLinesEx(hpBarBack, 1, BLACK);

            // ATK & DEF
            DrawText(("ATK: " + std::to_string(player.attack)).c_str(), 300, 170, 20, BLACK);
            DrawText(("DEF: " + std::to_string(player.defense)).c_str(), 300, 200, 20, BLACK);

            // EXP Text & Bar
            DrawText(("EXP: " + std::to_string(player.exp) + "/" + std::to_string(player.expToLevel)).c_str(), 300, 230, 20, BLACK);
            float expPercent = (float)player.exp / player.expToLevel;
            Rectangle expBarBack = { 440, 230, 200, 20 };
            Rectangle expBarFill = { 440, 230, 200 * expPercent, 20 };
            DrawRectangleRec(expBarBack, GRAY);
            DrawRectangleRec(expBarFill, DARKGREEN);
            DrawRectangleLinesEx(expBarBack, 1, BLACK);

            DrawText(("Coins: " + std::to_string(playerCoins)).c_str(), 300, 260, 20, BLACK);
            // change name button
            Color renameColor = CheckCollisionPointRec(mousePos, renameBtn) ? GRAY : DARKGOLD;
            DrawRectangleRec(renameBtn, renameColor);
            DrawRectangleLinesEx(renameBtn, 1, DARKGREEN);
            DrawText("Ganti Nama", (int)renameBtn.x + 20, (int)renameBtn.y + 7, 20, WHITE);
        }
        else if (currentMenu == SubMenu::Inventory) {
            DrawText("Inventory (Click or Press ENTER to use)", 60, 80, 25, DARKGREEN);

            int y = 120;
            int itemHeight = 30;
            for (size_t i = 0; i < inventory.size(); ++i) {
                Rectangle itemRect = { 60.0f, (float)y, 600.0f, (float)itemHeight };
                bool isHover = CheckCollisionPointRec(mousePos, itemRect);
                Color bgColor = (i == selectedItemIndex || isHover) ? DARKGOLD : CLITERAL(Color) { 220, 220, 220, 255 };
                DrawRectangleRec(itemRect, bgColor);
                DrawRectangleLinesEx(itemRect, 1, DARKGREEN);
                DrawText((inventory[i].name + " x" + std::to_string(inventory[i].quantity)).c_str(), 70, y + 5, 20, BLACK);

                if (isHover) selectedItemIndex = (int)i;
                if (isHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    UseItem(i);
                }

                y += itemHeight + 5;
            }

            if (IsKeyPressed(KEY_DOWN)) {
                selectedItemIndex = (selectedItemIndex + 1) % inventory.size();
            }
            else if (IsKeyPressed(KEY_UP)) {
                selectedItemIndex = (selectedItemIndex + inventory.size() - 1) % inventory.size();
            }
            else if (IsKeyPressed(KEY_ENTER) && !inventory.empty()) {
                UseItem(selectedItemIndex);
            }
        }
        else if (currentMenu == SubMenu::Skills) {
            DrawText("Skills (Select to Equip for Battle)", 60, 80, 25, DARKMAGENTA);

            int y = 120;
            int skillHeight = 30;
            if (playerSkills.empty()) {
                DrawText("You don't own any skills yet.", 70, y, 20, DARKGRAY);
            }
            else {
                for (size_t i = 0; i < playerSkills.size(); ++i) {
                    Rectangle skillRect = { 60.0f, (float)y, 600.0f, (float)skillHeight };
                    bool isHover = CheckCollisionPointRec(mousePos, skillRect);
                    Color bgColor = (i == selectedSkillIndex || isHover) ? DARKGOLD : CLITERAL(Color) { 220, 220, 220, 255 };
                    DrawRectangleRec(skillRect, bgColor);
                    DrawRectangleLinesEx(skillRect, 1, DARKMAGENTA);

                    std::string skillText = playerSkills[i].name + " - " + playerSkills[i].description;
                    if ((int)i == equippedSkillIndex) skillText += " [EQUIPPED]";
                    DrawText(skillText.c_str(), 70, y + 5, 20, BLACK);

                    if (isHover) selectedSkillIndex = (int)i;
                    if (isHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        equippedSkillIndex = (int)i;
                        ShowNotification("Equipped skill: " + playerSkills[i].name);
                    }

                    y += skillHeight + 5;
                }

                if (IsKeyPressed(KEY_DOWN)) {
                    selectedSkillIndex = (selectedSkillIndex + 1) % playerSkills.size();
                }
                else if (IsKeyPressed(KEY_UP)) {
                    selectedSkillIndex = (selectedSkillIndex + playerSkills.size() - 1) % playerSkills.size();
                }
                else if (IsKeyPressed(KEY_ENTER)) {
                    equippedSkillIndex = selectedSkillIndex;
                    ShowNotification("Equipped skill: " + playerSkills[equippedSkillIndex].name);
                }
            }
        }

        Rectangle backRect = { 20, static_cast<float>(screenHeight - 50), 180, 35 };
        Color backColor = CheckCollisionPointRec(mousePos, backRect) ? GRAY : DARKGOLD;
        DrawRectangleRec(backRect, backColor);
        DrawRectangleLinesEx(backRect, 2, DARKGREEN);
        DrawText("Back to Tavern", (int)backRect.x + 10, (int)backRect.y + 8, 20, WHITE);

        EndDrawing();

        if (CheckCollisionPointRec(mousePos, backRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            viewing = false;
        }
        if (CheckCollisionPointRec(mousePos, renameBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            std::string newName = EnterPlayerName();
            if (!newName.empty() && newName != player.name) {
                SetPlayerName(newName);
                SaveGame();
                ShowNotification("Nama berhasil diubah!");
            }
        }

        if (IsKeyPressed(KEY_TAB)) {
            // Cycle through Stats -> Inventory -> Skills
            if (currentMenu == SubMenu::Stats) currentMenu = SubMenu::Inventory;
            else if (currentMenu == SubMenu::Inventory) currentMenu = SubMenu::Skills;
            else currentMenu = SubMenu::Stats;
        }
        else if (IsKeyPressed(KEY_ESCAPE)) {
            viewing = false;
        }
    }

    // At the end of ShowPlayerStatsAndInventory()
    if (equippedSkillIndex >= 0 && equippedSkillIndex < (int)playerSkills.size()) {
        this->equippedSkillIndex = equippedSkillIndex;
    }
}

void Game::ShowTrainingGround() {
    state = GameState::TrainingGround;
    while (state == GameState::TrainingGround && !WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Training Ground (Coming Soon)", 20, 20, 30, DARKGRAY);
        DrawText("Press any key or click to return to Town.", 20, 80, 20, DARKGRAY);
        EndDrawing();

        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            ShowTownSquare();
            return;
        }
    }
}



void Game::UseItem(size_t index) {
    if (index >= inventory.size()) return;
    auto& item = inventory[index];
    bool used = false;

    if (item.name == "Potion") {
        if (player.currentHP < player.maxHP) {
            player.currentHP += 20;
            if (player.currentHP > player.maxHP) player.currentHP = player.maxHP;
            used = true;
            ShowNotification("You used a Potion!");
        }
        else {
            ShowNotification("HP is already full!");
        }
    }
    else if (item.name == "Hi-Potion") {
        if (player.currentHP < player.maxHP) {
            player.currentHP += 50;
            if (player.currentHP > player.maxHP) player.currentHP = player.maxHP;
            used = true;
            ShowNotification("You used a Hi-Potion!");
        }
        else {
            ShowNotification("HP is already full!");
        }
    }
    else if (item.name == "Elixir") {
        if (player.currentHP < player.maxHP) {
            player.currentHP = player.maxHP;
            used = true;
            ShowNotification("You used an Elixir!");
        }
        else {
            ShowNotification("HP is already full!");
        }
    }
    else if (item.name == "Antidote") {
        // Implement poison status if you have it, for now just show notification
        used = true;
        ShowNotification("You used an Antidote! (No poison implemented)");
    }
    else if (item.name == "Attack Up") {
        player.attack += 5;
        used = true;
        ShowNotification("Attack increased for next battle!");
    }
    else if (item.name == "Defense Up") {
        player.defense += 5;
        used = true;
        ShowNotification("Defense increased for next battle!");
    }
    else if (item.name == "Revive") {
        if (player.currentHP <= 0) {
            player.currentHP = player.maxHP / 2;
            used = true;
            ShowNotification("You have been revived!");
        }
        else {
            ShowNotification("You can't use Revive unless defeated!");
        }
    }
    else if (item.name == "Speed Boots") {
        used = true;
        ShowNotification("Speed increased for next battle! (Effect not implemented)");
    }
    else if (item.name == "Magic Water") {
        if (skillOnCooldown) {
            skillOnCooldown = false;
            skillCooldownTurns = 0;
            used = true;
            ShowNotification("Skill cooldown reset!");
        }
        else {
            ShowNotification("Skill is not on cooldown!");
        }
    }

    if (used) {
        item.quantity--;
        if (item.quantity <= 0) {
            inventory.erase(inventory.begin() + index);
        }
    }
}


void Game::ShowShop() {
    state = GameState::Shop;
    int selected = 0;
    int itemsPerPage = 5;
    int currentPage = 0;
    int totalPages = (shopItemCount + itemsPerPage - 1) / itemsPerPage;
    Rectangle backBtn = { 20.0f, static_cast<float>(screenHeight) - 80.0f, 150.0f, 40.0f };
    Rectangle prevBtn = { 200.0f, static_cast<float>(screenHeight) - 80.0f, 120.0f, 40.0f };
    Rectangle nextBtn = { 340.0f, static_cast<float>(screenHeight) - 80.0f, 100.0f, 40.0f };

    double shopEnterTime = GetTime();

    while (state == GameState::Shop && !WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Shop", 20, 20, 30, DARKPURPLE);
        std::string coinsText = "Coins: " + std::to_string(playerCoins);
        DrawText(coinsText.c_str(), 20, 60, 20, DARKGREEN);

        int y = 100;
        int itemHeight = 40;
        Vector2 mousePos = GetMousePosition();

        int startIdx = currentPage * itemsPerPage;
        int endIdx = std::min(startIdx + itemsPerPage, shopItemCount);

        for (int i = startIdx; i < endIdx; ++i) {
            int displayIdx = i - startIdx;
            Rectangle itemRect = { 20.0f, (float)y, 500.0f, (float)itemHeight };
            bool isHover = CheckCollisionPointRec(mousePos, itemRect);
            Color clr = (displayIdx == selected || isHover) ? GOLD : BLACK;
            std::string itemText = shopItems[i].name + " (" + std::to_string(shopItems[i].price) + " coins) - " + shopItems[i].description;
            DrawText(itemText.c_str(), 20, y, 20, clr);

            if (isHover) selected = displayIdx;

            if (isHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && (GetTime() - shopEnterTime > 1.0)) {
                if (playerCoins >= shopItems[i].price) {
                    bool found = false;
                    for (auto& item : inventory) {
                        if (item.name == shopItems[i].name) {
                            item.quantity++;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        inventory.push_back({ shopItems[i].name, shopItems[i].description, 1 });
                    }
                    playerCoins -= shopItems[i].price;
                    ShowNotification("Bought " + shopItems[i].name + "!");
                }
                else {
                    ShowNotification("Not enough coins!");
                }
            }
            y += itemHeight;
        }

        Color backColor = CheckCollisionPointRec(mousePos, backBtn) ? GRAY : DARKGRAY;
        DrawRectangleRec(backBtn, backColor);
        DrawText("Back", (int)backBtn.x + 10, (int)backBtn.y + 10, 20, WHITE);

        Color nextColor = (currentPage < totalPages - 1 && CheckCollisionPointRec(mousePos, nextBtn)) ? GRAY : DARKGRAY;
        DrawRectangleRec(nextBtn, nextColor);
        DrawText("Next", (int)nextBtn.x + 10, (int)nextBtn.y + 10, 20, WHITE);

        Color prevColor = (currentPage > 0 && CheckCollisionPointRec(mousePos, prevBtn)) ? GRAY : DARKGRAY;
        DrawRectangleRec(prevBtn, prevColor);
        DrawText("Previous", (int)prevBtn.x + 10, (int)prevBtn.y + 10, 20, WHITE);

        std::string pageText = "Page " + std::to_string(currentPage + 1) + " / " + std::to_string(totalPages);
        DrawText(pageText.c_str(), 480, screenHeight - 70, 20, DARKGRAY);

        DrawText("Buy: Enter | Back: ESC or Button", 20, screenHeight - 120, 20, DARKGRAY);

        if (GetTime() - shopEnterTime <= 1.0) {
            DrawText("Please wait...", 350, 60, 20, RED);
        }

        EndDrawing();

        if (IsKeyPressed(KEY_DOWN)) {
            selected = (selected + 1) % (endIdx - startIdx);
        }
        if (IsKeyPressed(KEY_UP)) {
            selected = (selected + (endIdx - startIdx) - 1) % (endIdx - startIdx);
        }
        if (IsKeyPressed(KEY_ENTER) && (GetTime() - shopEnterTime > 1.0)) {
            int i = startIdx + selected;
            if (playerCoins >= shopItems[i].price) {
                bool found = false;
                for (auto& item : inventory) {
                    if (item.name == shopItems[i].name) {
                        item.quantity++;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    inventory.push_back({ shopItems[i].name, shopItems[i].description, 1 });
                }
                playerCoins -= shopItems[i].price;
                ShowNotification("Bought " + shopItems[i].name + "!");
            }
            else {
                ShowNotification("Not enough coins!");
            }
        }
        if (currentPage < totalPages - 1 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, nextBtn)) {
            currentPage++;
            selected = 0;
        }
        if (currentPage > 0 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, prevBtn)) {
            currentPage--;
            selected = 0;
        }
        if (IsKeyPressed(KEY_RIGHT) && currentPage < totalPages - 1) {
            currentPage++;
            selected = 0;
        }
        if (IsKeyPressed(KEY_LEFT) && currentPage > 0) {
            currentPage--;
            selected = 0;
        }
        if (IsKeyPressed(KEY_ESCAPE) ||
            (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, backBtn))) {
            state = GameState::Market;
            return;
        }
    }
}

void Game::ShowSkillShop() {
    int selected = 0;
    Rectangle backBtn = { 20.0f, static_cast<float>(screenHeight) - 80.0f, 150.0f, 40.0f };

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Arcane Skill Emporium", 20, 20, 30, DARKMAGENTA);
        DrawText(("Coins: " + std::to_string(playerCoins)).c_str(), 20, 60, 20, DARKGREEN);

        int y = 100;
        for (size_t i = 0; i < availableSkills.size(); ++i) {
            Color color = (i == selected) ? GOLD : BLACK;
            std::string skillText = availableSkills[i].name + " (" +
                std::to_string(availableSkills[i].price) + " coins) - " +
                availableSkills[i].description +
                (availableSkills[i].owned ? " [Owned]" : "");
            DrawText(skillText.c_str(), 40, y, 22, color);
            y += 40;
        }

        // Draw Back button
        Vector2 mousePos = GetMousePosition();
        Color backColor = CheckCollisionPointRec(mousePos, backBtn) ? GRAY : DARKGRAY;
        DrawRectangleRec(backBtn, backColor);
        DrawText("Back", (int)backBtn.x + 10, (int)backBtn.y + 10, 20, WHITE);

        DrawText("Buy: Enter | Back: ESC or Button", 20, y + 20, 20, DARKGRAY);
        EndDrawing();

        if (IsKeyPressed(KEY_DOWN)) selected = (selected + 1) % availableSkills.size();
        if (IsKeyPressed(KEY_UP)) selected = (selected + availableSkills.size() - 1) % availableSkills.size();
        if (IsKeyPressed(KEY_ENTER)) {
            Skill& skill = availableSkills[selected];
            if (skill.owned) {
                ShowNotification("You already own this skill!");
            }
            else if (playerCoins >= skill.price) {
                playerCoins -= skill.price;
                skill.owned = true;
                playerSkills.push_back(skill);
                ShowNotification("You bought " + skill.name + "!");
            }
            else {
                ShowNotification("Not enough coins!");
            }
        }
        // Back with ESC or button
        if (IsKeyPressed(KEY_ESCAPE) ||
            (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, backBtn))) {
            break;
        }
    }
}

void Game::ShowSkillsMenu() {
    bool viewing = true;
    int selectedSkillIndex = 0;
    Rectangle backRect = { 20, static_cast<float>(screenHeight - 50), 180, 35 };

    while (viewing && !WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("Skills (Select to Equip for Battle)", 60, 40, 28, DARKMAGENTA);

        Vector2 mousePos = GetMousePosition();

        int y = 100;
        int skillHeight = 35;
        if (playerSkills.empty()) {
            DrawText("You don't own any skills yet.", 70, y, 22, DARKGRAY);
        }
        else {
            for (size_t i = 0; i < playerSkills.size(); ++i) {
                Rectangle skillRect = { 60.0f, (float)y, 600.0f, (float)skillHeight };
                bool isHover = CheckCollisionPointRec(mousePos, skillRect);
                Color bgColor = (i == selectedSkillIndex || isHover) ? DARKGOLD : CLITERAL(Color) { 220, 220, 220, 255 };
                DrawRectangleRec(skillRect, bgColor);
                DrawRectangleLinesEx(skillRect, 1, DARKMAGENTA);

                std::string skillText = playerSkills[i].name + " - " + playerSkills[i].description;
                if ((int)i == equippedSkillIndex) skillText += " [EQUIPPED]";
                DrawText(skillText.c_str(), 70, y + 7, 20, BLACK);

                if (isHover) selectedSkillIndex = (int)i;
                if (isHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    equippedSkillIndex = (int)i;
                    ShowNotification("Equipped skill: " + playerSkills[i].name);
                }

                y += skillHeight + 8;
            }

            // Keyboard navigation
            if (IsKeyPressed(KEY_DOWN)) {
                selectedSkillIndex = (selectedSkillIndex + 1) % playerSkills.size();
            }
            else if (IsKeyPressed(KEY_UP)) {
                selectedSkillIndex = (selectedSkillIndex + playerSkills.size() - 1) % playerSkills.size();
            }
            else if (IsKeyPressed(KEY_ENTER)) {
                equippedSkillIndex = selectedSkillIndex;
                ShowNotification("Equipped skill: " + playerSkills[equippedSkillIndex].name);
            }
        }

        // Draw Back button
        Color backColor = CheckCollisionPointRec(mousePos, backRect) ? GRAY : DARKGOLD;
        DrawRectangleRec(backRect, backColor);
        DrawRectangleLinesEx(backRect, 2, DARKGREEN);
        DrawText("Back", (int)backRect.x + 10, (int)backRect.y + 8, 20, WHITE);

        EndDrawing();

        if (CheckCollisionPointRec(mousePos, backRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            viewing = false;
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            viewing = false;
        }
    }
}

void Game::StartBattle() {
    state = GameState::Battle;
    selectedAction = 0;
    attackEffectFrame = 0;
    showAttackEffect = false;
    isPlayerTurn = true;
    isBlocking = false;
    skillCooldownTurns = 0; // Reset cooldown saat mulai battle
    skillOnCooldown = false;
    battleLog.clear();

    while (state == GameState::Battle && !WindowShouldClose()) {
        UpdateBattle();

        BeginDrawing();
        ClearBackground(BEIGE);
        DrawBattle();
        DrawAttackEffect();
        EndDrawing();
    }
}

void Game::UpdateBattle() {
    const char* actions[5] = { "Attack", "Skill", "Block", "Item", "Run" };
    Vector2 mousePos = GetMousePosition();

    // Keyboard navigation
    if (IsKeyPressed(KEY_DOWN)) {
        do {
            selectedAction = (selectedAction + 1) % 5;
        } while (selectedAction == 1 && skillOnCooldown); // Skip Skill if on cooldown
    }
    else if (IsKeyPressed(KEY_UP)) {
        do {
            selectedAction = (selectedAction + 4) % 5;
        } while (selectedAction == 1 && skillOnCooldown); // Skip Skill if on cooldown
    }
    else if (IsKeyPressed(KEY_ENTER)) {
        if (isPlayerTurn && !(selectedAction == 1 && skillOnCooldown)) {
            PerformPlayerAction(selectedAction);
            CheckBattleResult();
            if (state != GameState::Battle)
                return;
            isPlayerTurn = false;
        }
    }



    // Mouse click support for action selection
    for (int i = 0; i < 5; i++) {
        Rectangle actionRect = { 20.0f, static_cast<float>(screenHeight - 150 + i * 30), MeasureText(actions[i], 20), 30.0f };
        if (CheckCollisionPointRec(mousePos, actionRect)) {
            if (!(i == 1 && skillOnCooldown)) { // Only allow hover/select if not disabled
                selectedAction = i;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && isPlayerTurn) {
                    PerformPlayerAction(selectedAction);
                    CheckBattleResult();
                    if (state != GameState::Battle)
                        return;
                    isPlayerTurn = false;
                    break;
                }
            }
        }
    }

    if (!isPlayerTurn && state == GameState::Battle) {
        if (!skipEnemyTurn) {
            EnemyAttack();
            CheckBattleResult();
            if (state != GameState::Battle)
                return;

            // === Turunkan cooldown player skill ===
            if (skillOnCooldown) {
                skillCooldownTurns--;
                if (skillCooldownTurns <= 0) {
                    skillOnCooldown = false;
                    ShowNotification("Skill ready to use!");
                }
            }

            // === Turunkan cooldown musuh skill ===
            if (enemySkillCooldown > 0) {
                enemySkillCooldown--;
            }
        }
        isPlayerTurn = true;
        skipEnemyTurn = false;
    }
    bool canUseSkill = !skillOnCooldown && equippedSkillIndex >= 0 && equippedSkillIndex < (int)playerSkills.size();
    if (IsKeyPressed(KEY_DOWN)) {
        do {
            selectedAction = (selectedAction + 1) % 5;
        } while (selectedAction == 1 && !canUseSkill);
    }
    else if (IsKeyPressed(KEY_UP)) {
        do {
            selectedAction = (selectedAction + 4) % 5;
        } while (selectedAction == 1 && !canUseSkill);
    }



    if (showAttackEffect) {
        attackEffectFrame++;
        if (attackEffectFrame > 30) {
            showAttackEffect = false;
            attackEffectFrame = 0;
        }
    }
}


void Game::DrawBattle() {
    std::string playerLvl = player.name + " - Lvl " + std::to_string(player.level);
    DrawText(playerLvl.c_str(), 20, 20, 20, DARKBLUE);

    std::string playerHP = "HP: " + std::to_string(player.currentHP) + "/" + std::to_string(player.maxHP);
    DrawText(playerHP.c_str(), 20, 50, 20, RED);

    std::string enemyLvl = enemy.name + " Lvl " + std::to_string(enemy.level);
    DrawText(enemyLvl.c_str(), screenWidth - 200, 20, 20, MAROON);

    std::string enemyHP = "HP: " + std::to_string(enemy.currentHP) + "/" + std::to_string(enemy.maxHP);
    DrawText(enemyHP.c_str(), screenWidth - 200, 50, 20, RED);

    std::string expText = "EXP: " + std::to_string(player.exp) + "/" + std::to_string(player.expToLevel);
    DrawText(expText.c_str(), 20, 80, 20, DARKMAGENTA);

    const char* actions[5] = { "Attack", "Skill", "Block", "Item", "Run" };
    Vector2 mousePos = GetMousePosition();

    // Draw battle log (bottom right)
    int logFontSize = 16;               // Sedikit lebih besar agar lebih mudah dibaca
    int logLineHeight = 22;            // Kurangi jarak antar baris
    int logBoxWidth = 300;             // Lebarkan box agar lebih lapang
    int logBoxHeight = BATTLE_LOG_MAX_LINES * logLineHeight + 30;
    int logBoxX = screenWidth - logBoxWidth - 20;
    int logBoxY = screenHeight - logBoxHeight - 20;

    // Draw background box
    DrawRectangle(logBoxX, logBoxY, logBoxWidth, logBoxHeight, Fade(DARKGRAY, 0.7f));

    // Draw log title
    DrawText("Battle Log", logBoxX + 10, logBoxY + 4, logFontSize, GOLD);

    // Draw log lines
    int y = logBoxY + 8 + logLineHeight;
    for (const auto& line : battleLog) {
        DrawText(line.c_str(), logBoxX + 10, y, logFontSize, WHITE);
        y += logLineHeight;
    }

    // Draw action box background
    int actionBoxX = 10;
    int actionBoxY = screenHeight - 160;
    int actionBoxWidth = 180;
    int actionBoxHeight = 5 * 30 + 20;
    DrawRectangle(actionBoxX, actionBoxY, actionBoxWidth, actionBoxHeight, Fade(DARKGRAY, 0.7f));

    // Draw action buttons
    for (int i = 0; i < 5; i++) {
        int actionY = screenHeight - 150 + i * 30;
        Color clr;
        bool isSkill = (i == 1);
        bool disabled = isSkill && skillOnCooldown;

        // Highlight if selected and not disabled, or mouse hover and not disabled
        Rectangle actionRect = { 20.0f, (float)actionY, MeasureText(actions[i], 20), 30.0f };
        bool isMouseHover = CheckCollisionPointRec(mousePos, actionRect);

        if (disabled) {
            clr = GRAY;
        }
        else if (isMouseHover) {
            clr = GOLD;
            selectedAction = i;
        }
        else {
            clr = (i == selectedAction) ? DARKGOLD : BLACK;
        }

        DrawText(actions[i], (int)actionRect.x, (int)actionRect.y, 20, clr);

        // Draw cooldown info next to Skill
        if (isSkill && skillOnCooldown) {
            std::string cooldownText = " (" + std::to_string(skillCooldownTurns) + ")";
            int actionX = 20 + MeasureText("Skill", 20) + 10;
            DrawText(cooldownText.c_str(), actionX, actionY, 20, DARKRED);
        }
    }



    if (skillOnCooldown) {
        int skillIdx = 1;
        int actionY = screenHeight - 150 + skillIdx * 30;
        int actionX = 20 + MeasureText("Skill", 20) + 10;
        std::string cooldownText = " (" + std::to_string(skillCooldownTurns) + ")";
        DrawText(cooldownText.c_str(), actionX, actionY, 20, DARKRED);
    }
}

void Game::ShowBattleItemMenu() {
    if (inventory.empty()) {
        ShowNotification("You have no items!");
        return;
    }

    int selected = 0;
    int itemsPerPage = 5;
    int currentPage = 0;
    int totalPages = (inventory.size() + itemsPerPage - 1) / itemsPerPage;
    Rectangle backBtn = { 20.0f, static_cast<float>(screenHeight) - 80.0f, 150.0f, 40.0f };
    Rectangle prevBtn = { 200.0f, static_cast<float>(screenHeight) - 80.0f, 120.0f, 40.0f };
    Rectangle nextBtn = { 340.0f, static_cast<float>(screenHeight) - 80.0f, 100.0f, 40.0f };

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Choose an item:", 20, 20, 24, DARKBLUE);

        int y = 60;
        int itemHeight = 40;
        Vector2 mousePos = GetMousePosition();

        int startIdx = currentPage * itemsPerPage;
        int endIdx = std::min(startIdx + itemsPerPage, (int)inventory.size());

        // Draw items for current page
        for (int i = startIdx; i < endIdx; ++i) {
            int displayIdx = i - startIdx;
            Rectangle itemRect = { 20.0f, (float)y, 500.0f, (float)itemHeight };
            bool isHover = CheckCollisionPointRec(mousePos, itemRect);
            Color clr = (displayIdx == selected || isHover) ? GOLD : BLACK;
            std::string itemText = inventory[i].name + " x" + std::to_string(inventory[i].quantity) + " - " + inventory[i].description;
            DrawText(itemText.c_str(), 20, y, 20, clr);

            if (isHover) selected = displayIdx;

            // Mouse click to use item
            if (isHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                UseItem(i);
                EndDrawing();
                return;
            }
            y += itemHeight;
        }

        // Draw Back button
        Color backColor = CheckCollisionPointRec(mousePos, backBtn) ? GRAY : DARKGRAY;
        DrawRectangleRec(backBtn, backColor);
        DrawText("Back", (int)backBtn.x + 10, (int)backBtn.y + 10, 20, WHITE);

        // Draw Next button
        Color nextColor = (currentPage < totalPages - 1 && CheckCollisionPointRec(mousePos, nextBtn)) ? GRAY : DARKGRAY;
        DrawRectangleRec(nextBtn, nextColor);
        DrawText("Next", (int)nextBtn.x + 10, (int)nextBtn.y + 10, 20, WHITE);

        // Draw Previous button
        Color prevColor = (currentPage > 0 && CheckCollisionPointRec(mousePos, prevBtn)) ? GRAY : DARKGRAY;
        DrawRectangleRec(prevBtn, prevColor);
        DrawText("Previous", (int)prevBtn.x + 10, (int)prevBtn.y + 10, 20, WHITE);

        // Page indicator
        std::string pageText = "Page " + std::to_string(currentPage + 1) + " / " + std::to_string(totalPages);
        DrawText(pageText.c_str(), 480, screenHeight - 70, 20, DARKGRAY);

        DrawText("Use: Enter/Click | Back: ESC or Button", 20, screenHeight - 120, 20, DARKGRAY);

        EndDrawing();

        // Keyboard navigation
        if (IsKeyPressed(KEY_DOWN)) {
            selected = (selected + 1) % (endIdx - startIdx);
        }
        if (IsKeyPressed(KEY_UP)) {
            selected = (selected + (endIdx - startIdx) - 1) % (endIdx - startIdx);
        }
        if (IsKeyPressed(KEY_ENTER)) {
            UseItem(startIdx + selected);
            return;
        }
        // Next/Previous page with mouse
        if (currentPage < totalPages - 1 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, nextBtn)) {
            currentPage++;
            selected = 0;
        }
        if (currentPage > 0 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, prevBtn)) {
            currentPage--;
            selected = 0;
        }
        // Next/Previous page with keyboard
        if (IsKeyPressed(KEY_RIGHT) && currentPage < totalPages - 1) {
            currentPage++;
            selected = 0;
        }
        if (IsKeyPressed(KEY_LEFT) && currentPage > 0) {
            currentPage--;
            selected = 0;
        }
        // Back button or ESC
        if (IsKeyPressed(KEY_ESCAPE) ||
            (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, backBtn))) {
            break;
        }
    }
}


void Game::DrawAttackEffect() {
    if (showAttackEffect) {
        int alpha = 255 - (attackEffectFrame * 8);
        DrawText("Attack!", screenWidth / 2 - 50, screenHeight / 2, 40, Fade(RED, alpha / 255.0f));
    }
}


void Game::ShowVictoryScreen(int expGain, int coinGain, const std::string& enemyName) {
    // Prepare strings and font sizes
    std::string title = "Victory!";
    int titleFontSize = 40;
    std::string msg = "You defeated the " + enemyName + "!";
    int msgFontSize = 28;
    std::string reward = "You get " + std::to_string(expGain) + " EXP and " + std::to_string(coinGain) + " coins.";
    int rewardFontSize = 24;
    std::string prompt = "Press Enter to return to Arena";
    int promptFontSize = 22;

    // Calculate Y positions
    int y = 100;
    int spacing = 20;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(DARKGREEN);

        // Centered Title
        int titleWidth = MeasureText(title.c_str(), titleFontSize);
        DrawText(title.c_str(), this->screenWidth / 2 - titleWidth / 2, y, titleFontSize, GOLD);

        // Centered Message
        int msgWidth = MeasureText(msg.c_str(), msgFontSize);
        DrawText(msg.c_str(), this->screenWidth / 2 - msgWidth / 2, y + titleFontSize + spacing, msgFontSize, WHITE);

        // Centered Reward
        int rewardWidth = MeasureText(reward.c_str(), rewardFontSize);
        DrawText(reward.c_str(), this->screenWidth / 2 - rewardWidth / 2, y + titleFontSize + spacing + msgFontSize + spacing, rewardFontSize, YELLOW);

        // Centered Prompt
        int promptWidth = MeasureText(prompt.c_str(), promptFontSize);
        DrawText(prompt.c_str(), this->screenWidth / 2 - promptWidth / 2, y + titleFontSize + spacing + msgFontSize + spacing + rewardFontSize + spacing, promptFontSize, LIGHTGRAY);

        EndDrawing();

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) break;
    }
}

void Game::ShowDefeatScreen() {
    std::string title = "Defeat!";
    int titleFontSize = 40;
    std::string msg = "You have been defeated!";
    int msgFontSize = 28;
    std::string penalty = "You lost 5 coins.";
    int penaltyFontSize = 24;
    std::string prompt = "Press Enter to return to Arena";
    int promptFontSize = 22;

    int y = 100;
    int spacing = 20;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(DARKRED);

        // Centered Title
        int titleWidth = MeasureText(title.c_str(), titleFontSize);
        DrawText(title.c_str(), this->screenWidth / 2 - titleWidth / 2, y, titleFontSize, RED);

        // Centered Message
        int msgWidth = MeasureText(msg.c_str(), msgFontSize);
        DrawText(msg.c_str(), this->screenWidth / 2 - msgWidth / 2, y + titleFontSize + spacing, msgFontSize, WHITE);

        // Centered Penalty
        int penaltyWidth = MeasureText(penalty.c_str(), penaltyFontSize);
        DrawText(penalty.c_str(), this->screenWidth / 2 - penaltyWidth / 2, y + titleFontSize + spacing + msgFontSize + spacing, penaltyFontSize, YELLOW);

        // Centered Prompt
        int promptWidth = MeasureText(prompt.c_str(), promptFontSize);
        DrawText(prompt.c_str(), this->screenWidth / 2 - promptWidth / 2, y + titleFontSize + spacing + msgFontSize + spacing + penaltyFontSize + spacing, promptFontSize, LIGHTGRAY);

        EndDrawing();

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) break;
    }
}

void Game::PerformPlayerAction(int actionIndex) {
    ApplyPoisonDamageIfNeeded(); // ← tambahkan di awal
    Command* cmd = nullptr;


    switch (actionIndex) {
    case 0: // Attack
        cmd = new AttackCommand();
        break;

    case 1: // Skill
        if (skillOnCooldown) {
            ShowNotification("Skill on cooldown!");
            return;
        }
        if (equippedSkillIndex < 0 || equippedSkillIndex >= (int)playerSkills.size()) {
            ShowNotification("No skill equipped!");
            return;
        }
        UseEquippedSkill();
        skillOnCooldown = true;
        skillCooldownTurns = 3;
        isPlayerTurn = false;
        return;


    case 2: // Block
        cmd = new BlockCommand();
        break;

    case 3: // Item
        ShowBattleItemMenu();
        skipEnemyTurn = true; // Prevent enemy turn
        return;

    case 4: // Run
        cmd = new RunCommand();
        break;
    }

    if (cmd) {
        cmd->Execute(*this);
        delete cmd;
        isPlayerTurn = false; // hanya aksi selain Item yang akhiri giliran
    }
}

void Game::PlayerAttack() {
    int damage = player.attack - enemy.defense;
    if (enemyBlocking) damage /= 4; // Reduce damage if enemy is blocking
    if (damage < 1) damage = 1;
    enemy.currentHP -= damage;
    ShowNotification(" You attacks enemy for " + std::to_string(damage) + " damage!");

}

void Game::PlayerSkill() {
    int damage = (player.attack * 2) - enemy.defense;
    if (enemyBlocking) damage /= 4; // Reduce damage if enemy is blocking
    if (damage < 1) damage = 1;
    enemy.currentHP -= damage;
    ShowNotification("You uses skill for " + std::to_string(damage) + " damage!");
    showAttackEffect = true;
}

void Game::UseEquippedSkill() {
    if (equippedSkillIndex < 0 || equippedSkillIndex >= (int)playerSkills.size()) return;
    const Skill& skill = playerSkills[equippedSkillIndex];

    if (skill.name == "Blazing Strike") {
        int damage = (player.attack * 2) - enemy.defense + 5;
        if (enemyBlocking) damage /= 4;
        if (damage < 1) damage = 1;
        enemy.currentHP -= damage;
        ShowNotification("You unleash Blazing Strike for " + std::to_string(damage) + " fire damage!");
    }
    else if (skill.name == "Frost Guard") {
        // Example: Reduce damage for 2 turns (implement your own logic)
        isBlocking = true;
        ShowNotification("You use Frost Guard! Incoming damage reduced for 2 turns.");
    }
    else if (skill.name == "Thunder Dash") {
        int damage = (player.attack * 1.5) - enemy.defense + 3;
        if (enemyBlocking) damage /= 4;
        if (damage < 1) damage = 1;
        enemy.currentHP -= damage;
        ShowNotification("You dash with thunder for " + std::to_string(damage) + " damage!");
    }
    else {
        // Default fallback
        int damage = (player.attack * 2) - enemy.defense;
        if (enemyBlocking) damage /= 4;
        if (damage < 1) damage = 1;
        enemy.currentHP -= damage;
        ShowNotification("You use your skill for " + std::to_string(damage) + " damage!");
    }
    showAttackEffect = true;
}


EnemyAction Game::ChooseEnemyAction() const {
    float enemyHpPercent = static_cast<float>(enemy.currentHP) / enemy.maxHP;
    float playerHpPercent = static_cast<float>(player.currentHP) / player.maxHP;

    int scoreAttack = 10;
    int scoreBlock = 5;
    int scoreSkill = 0;

    // === Penyesuaian berdasarkan HP ===
    if (enemyHpPercent < 0.5f) scoreBlock += 2;
    if (enemyHpPercent < 0.3f) scoreBlock += 3;

    if (playerHpPercent < 0.3f) scoreAttack += 5;
    if (playerHpPercent > 0.8f) scoreSkill += 2;

    // === Hindari spam block ===
    if (lastEnemyAction == EnemyAction::Block) scoreBlock -= 6;

    // === Cek cooldown skill ===
    if (enemySkillCooldown > 0) {
        scoreSkill = -100; // abaikan opsi skill
    }
    else {
        // === Logika skill berdasarkan tipe musuh ===
        switch (enemyType) {
        case EnemyType::Archer:
            scoreSkill += 8 + GetRandom(0, 2); // sering gunakan skill
            break;
        case EnemyType::Warrior:
            if (enemyHpPercent < 0.6f) scoreSkill += 10;
            break;
        case EnemyType::Paladin:
            if (lastEnemyAction == EnemyAction::Block) scoreSkill += 12;
            break;
        case EnemyType::Witch:
            if (!playerPoisoned) scoreSkill += 15;
            break;
        }
    }

    // === Randomizer ringan untuk variasi tak terduga ===
    scoreAttack += GetRandom(0, 2);
    scoreBlock += GetRandom(0, 2);
    scoreSkill += GetRandom(0, 2);

    // === Pilih aksi dengan skor tertinggi ===
    if (scoreSkill >= scoreAttack && scoreSkill >= scoreBlock) return EnemyAction::Skill;
    if (scoreAttack >= scoreBlock) return EnemyAction::Attack;
    return EnemyAction::Block;
}


void Game::ApplyPoisonDamageIfNeeded() {
    if (playerPoisoned) {
        int poisonDmg = std::max(1, player.maxHP * 5 / 100);
        player.currentHP -= poisonDmg;
        ShowNotification("Poison deals " + std::to_string(poisonDmg) + " damage!");
        poisonTurns--;
        if (poisonTurns <= 0) {
            playerPoisoned = false;
            ShowNotification("You are no longer poisoned!");
        }
    }
}


void Game::EnemyAttack() {
    if (enemy.currentHP <= 0) return;


    EnemyAction action = ChooseEnemyAction(); // Pilih aksi via AI
    int damage = 0;

    switch (action) {
    case EnemyAction::Attack:
        damage = enemy.attack - player.defense;
        if (isBlocking) damage /= 4;
        player.currentHP -= std::max(1, damage);
        ShowNotification(enemy.name + " attacks for " + std::to_string(damage) + " damage!");
        break;

    case EnemyAction::Block:
        enemyBlocking = true;
        ShowNotification(enemy.name + " is blocking!");
        break;

    case EnemyAction::Skill:
        enemySkillCooldown = 3;
        if (enemyType == EnemyType::Paladin) {
            damage = (enemy.attack * 1.5) - player.defense;
            player.currentHP -= std::max(1, damage);
            ShowNotification(enemy.name + " uses Holy Strike for " + std::to_string(damage) + " damage!");
        }
        else if (enemyType == EnemyType::Archer) {
            damage = (enemy.attack * 2) - player.defense;
            player.currentHP -= std::max(1, damage);
            ShowNotification(enemy.name + " uses Double Shot for " + std::to_string(damage) + " damage!");
        }
        else if (enemyType == EnemyType::Warrior) {
            damage = (enemy.attack * 2) - player.defense;
            player.currentHP -= std::max(1, damage);
            ShowNotification(enemy.name + " uses Power Strike for " + std::to_string(damage) + " damage!");
        }
        else if (enemyType == EnemyType::Witch) {
            playerPoisoned = true;
            poisonTurns = 3;
            ShowNotification(enemy.name + " uses Poison! You are poisoned for 3 turns.");
        }
        break;
    }

    lastEnemyAction = action;
    showAttackEffect = true;
}

void Game::CheckBattleResult() {
    // Pastikan HP tidak negatif
    player.currentHP = std::max(0, player.currentHP);
    enemy.currentHP = std::max(0, enemy.currentHP);

    // Player kalah
    if (player.currentHP == 0) {
        ShowNotification("You have been defeated! Lose 5 coins.");
        playerCoins = std::max(0, playerCoins - 5);
        ShowDefeatScreen(); // Show defeat scene
        player.currentHP = player.maxHP;  // Reset HP
        state = GameState::Arena;        // Return to arena
        return;
    }


    // Enemy kalah
    if (enemy.currentHP == 0) {
        int expGain = baseEnemyExp;
        int coinGain = baseEnemyCoins;
        playerCoins += coinGain;
        player.exp += expGain;
        ShowVictoryScreen(expGain, coinGain, enemy.name);

        // Level up player
        while (player.exp >= player.expToLevel) {
            player.level++;
            player.exp -= player.expToLevel;
            player.expToLevel = static_cast<int>(player.expToLevel * 1.2f);
            player.maxHP += 10;
            player.attack += 2;
            player.defense += 1;
            ShowNotification("Level up! Now level " + std::to_string(player.level));
        }

        ShowVictoryScreen(expGain, coinGain, enemy.name);
        state = GameState::Arena;
        enemyLevel++; // Tingkatkan level enemy berikutnya
    }
}

int Game::GetRandom(int min, int max) {
    return min + rand() % (max - min + 1);
}

void Game::SaveGame() {
    std::ofstream out("save.dat", std::ios::binary);
    if (!out) return;

    // Save player name length and name
    size_t nameLen = player.name.size();
    out.write(reinterpret_cast<const char*>(&nameLen), sizeof(size_t));
    out.write(player.name.c_str(), nameLen);

    // Save player stats
    out.write(reinterpret_cast<const char*>(&player.maxHP), sizeof(int));
    out.write(reinterpret_cast<const char*>(&player.currentHP), sizeof(int));
    out.write(reinterpret_cast<const char*>(&player.attack), sizeof(int));
    out.write(reinterpret_cast<const char*>(&player.defense), sizeof(int));
    out.write(reinterpret_cast<const char*>(&player.level), sizeof(int));
    out.write(reinterpret_cast<const char*>(&playerCoins), sizeof(int));
    out.write(reinterpret_cast<const char*>(&player.exp), sizeof(int));
    out.write(reinterpret_cast<const char*>(&player.expToLevel), sizeof(int));

    // Save inventory
    size_t invSize = inventory.size();
    out.write(reinterpret_cast<const char*>(&invSize), sizeof(size_t));
    for (const auto& item : inventory) {
        size_t nameLen = item.name.size();
        size_t descLen = item.description.size();
        out.write(reinterpret_cast<const char*>(&nameLen), sizeof(size_t));
        out.write(item.name.c_str(), nameLen);
        out.write(reinterpret_cast<const char*>(&descLen), sizeof(size_t));
        out.write(item.description.c_str(), descLen);
        out.write(reinterpret_cast<const char*>(&item.quantity), sizeof(int));
    }

    out.close();
}

void Game::LoadGame() {
    std::ifstream in("save.dat", std::ios::binary);
    if (!in) return;

    // Load player name length and name
    size_t nameLen = 0;
    in.read(reinterpret_cast<char*>(&nameLen), sizeof(size_t));
    player.name.resize(nameLen);
    in.read(&player.name[0], nameLen);

    // Load player stats
    in.read(reinterpret_cast<char*>(&player.maxHP), sizeof(int));
    in.read(reinterpret_cast<char*>(&player.currentHP), sizeof(int));
    in.read(reinterpret_cast<char*>(&player.attack), sizeof(int));
    in.read(reinterpret_cast<char*>(&player.defense), sizeof(int));
    in.read(reinterpret_cast<char*>(&player.level), sizeof(int));
    in.read(reinterpret_cast<char*>(&playerCoins), sizeof(int));
    in.read(reinterpret_cast<char*>(&player.exp), sizeof(int));
    in.read(reinterpret_cast<char*>(&player.expToLevel), sizeof(int));

    // Load inventory
    size_t invSize = 0;
    in.read(reinterpret_cast<char*>(&invSize), sizeof(size_t));
    inventory.clear();
    for (size_t i = 0; i < invSize; ++i) {
        Item item;
        size_t nameLen = 0, descLen = 0;
        in.read(reinterpret_cast<char*>(&nameLen), sizeof(size_t));
        item.name.resize(nameLen);
        in.read(&item.name[0], nameLen);
        in.read(reinterpret_cast<char*>(&descLen), sizeof(size_t));
        item.description.resize(descLen);
        in.read(&item.description[0], descLen);
        in.read(reinterpret_cast<char*>(&item.quantity), sizeof(int));
        inventory.push_back(item);
    }
    in.close();
}
