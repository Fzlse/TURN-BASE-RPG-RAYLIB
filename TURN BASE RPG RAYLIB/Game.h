#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <string>
#include <vector>
#include "NotificationObserver.h"
#include "Command.h"
#include <deque>
#include <functional>


enum class GameState {
    MainMenu,
    TownSquare,
    Colosseum,
    QuickBattle,
    Market,
    Tavern,
    TrainingGround,
    Shop,
    Battle,
    Arena,
    Exit,
};

struct Character {
    std::string name;
    int maxHP;
    int currentHP;
    int attack;
    int defense;
    int level;
    int exp;
    int expToLevel;
};

struct Skill {
    std::string name;
    std::string description;
    int price;
    bool owned = false;
};

enum class EnemyAction {
    Attack,
    Block,
    Skill,
    Poison // For Witch
};

struct Item {
    std::string name;
    std::string description;
    int quantity;
};

enum class EnemyType {
    Archer,
    Warrior,
    Paladin,
    Witch // New enemy type
};

class Game {
public:
    Game(int screenWidth, int screenHeight);

    void ShowTownSquare();
    void ShowColosseum();
    void ShowMarket();
    void ShowTavern();
    void ShowTrainingGround();
    bool IsRunning() const;
    void ShowShop();
    void StartBattle();
    void ShowInventory();
    void ShowNotification(const std::string& msg);
    void UseItem(size_t index);
    void ShowBattleItemMenu();
    void ShowPlayerStatsAndInventory();
    void ShowSkillShop();
    void ShowSkillsMenu();
    void UseEquippedSkill();


    static const int BATTLE_LOG_MAX_LINES = 5;
    std::vector<std::string> battleLog;
    void Unload();
    void SetPlayerName(const std::string& name);
    std::string GetPlayerName() const;
    std::string EnterPlayerName();

    void SaveGame();
    void LoadGame();

    // Observer pattern methods
    void AddObserver(NotificationObserver* observer);
    void RemoveObserver(NotificationObserver* observer);

    // Command pattern method
    void PerformPlayerAction(int actionIndex);

    // Expose these for Command pattern
    void PlayerAttack();
    void PlayerSkill();

    // Expose for BlockCommand and RunCommand
    bool isBlocking;
    GameState state;

private:
    int GetRandom(int min, int max);

    void InitPlayer();
    Texture2D characterTexture;
    void InitEnemy();
    void InitEnemyForSurvival(int wave);
    int equippedSkillIndex = -1; // -1 means no skill equipped



    int enemyLevel;  // Level independen dari player
    int baseEnemyExp;  // EXP dasar yang diberikan enemy
    int baseEnemyCoins; // Koin dasar yang diberikan enemy
    bool enemyBlocking = false;
    EnemyAction lastEnemyAction = EnemyAction::Attack;
    int enemySkillCooldown = 0;

    bool playerPoisoned = false;
    int poisonTurns = 0;
    void ApplyPoisonDamageIfNeeded();

    // AI: Pilih aksi enemy secara adaptif
    EnemyAction ChooseEnemyAction() const;
    int GetRandom(int min, int max) const;

    Character player;
    Character enemy;
    EnemyType enemyType;
    std::vector<Item> inventory;

    void UpdateBattle();
    void DrawBattle();
    void DrawAttackEffect();

    std::vector<Skill> availableSkills;
    std::vector<Skill> playerSkills;


    void EnemyAttack();
    void CheckBattleResult();

    void ShowVictoryScreen(int expGain, int coinGain, const std::string& enemyName);
    void ShowDefeatScreen();





    int screenWidth;
    int screenHeight;

    bool running;

    int playerCoins;
    int selectedAction;

    int skillCooldownTurns;
    bool skillOnCooldown;

    std::string notificationText;
    int notificationTimer; // in frames

    bool showAttackEffect;
    int attackEffectFrame;

    bool isPlayerTurn;
    bool skipEnemyTurn = false;

    // Observer pattern
    std::vector<NotificationObserver*> observers;
};

#endif // GAME_H
