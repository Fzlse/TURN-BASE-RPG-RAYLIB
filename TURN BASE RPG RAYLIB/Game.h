#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <string>
#include <vector>
#include <deque>
#include <functional>
#include "NotificationObserver.h"
#include "Command.h"

// Enums
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

enum class EnemyAction {
    Attack,
    Block,
    Skill,
    Poison // For Witch
};

enum class EnemyType {
    Archer,
    Warrior,
    Paladin,
    Witch // New enemy type
};

// Structs
struct Character{
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

struct Item {
    std::string name;
    std::string description;
    int quantity;
};

// Game class
class Game {
public:
    Game(int screenWidth, int screenHeight);

    // Main game screens
    void ShowTownSquare();
    void ShowColosseum();
    void ShowMarket();
    void ShowTavern();
    void ShowTrainingGround();
    void ShowShop();
    void ShowInventory();
    void ShowSkillShop();
    void ShowSkillsMenu();
    void ShowPlayerStatsAndInventory();
    void ShowBattleItemMenu();
    void ShowNotification(const std::string& msg);
    void ShowDeveloperMenu();
    void ShowLoadingScreen(const std::string& message, std::function<void()> work);


    // Battle
    void StartBattle();
    void UseItem(size_t index);
    void UseEquippedSkill();
    void PlayerAttack();
    void PlayerSkill();
    void PerformPlayerAction(int actionIndex);

    // Game state
    bool IsRunning() const;
    void Unload();
    void SetPlayerName(const std::string& name);
    std::string GetPlayerName() const;
    std::string EnterPlayerName();

    // Save/Load
    void SaveGame();
    void LoadGame();

    // Observer pattern
    void AddObserver(NotificationObserver* observer);
    void RemoveObserver(NotificationObserver* observer);

    // Exposed for commands
    bool isBlocking;
    GameState state;

    // Battle log
    static const int BATTLE_LOG_MAX_LINES = 5;
    std::vector<std::string> battleLog;

private:
    // Initialization
    void InitPlayer();
    void InitEnemy();
    void InitEnemyForSurvival(int wave);

    // Enemy AI
    EnemyAction ChooseEnemyAction() const;
    int GetRandom(int min, int max) const;

    // Battle logic
    void UpdateBattle();
    void DrawBattle();
    void DrawAttackEffect();
    void EnemyAttack();
    void CheckBattleResult();
    void ApplyPoisonDamageIfNeeded();

    // Battle results
    void ShowVictoryScreen(int expGain, int coinGain, const std::string& enemyName);
    void ShowDefeatScreen();

    // Members
    int screenWidth;
    int screenHeight;
    bool running = true;

    Character player;
    Character enemy;
    EnemyType enemyType;

    std::vector<Item> inventory;
    std::vector<Skill> availableSkills;
    std::vector<Skill> playerSkills;


    // In class Game (private section)
    Texture2D characterTexture;
    Texture2D archerTexture;
    Texture2D warriorTexture;
    Texture2D paladinTexture;
    Texture2D witchTexture;
    Texture2D enemyTexture;
    Texture2D battleBgTexture;


    int equippedSkillIndex = -1;
    int enemyLevel;
    int baseEnemyExp;
    int baseEnemyCoins;
    bool enemyBlocking = false;
    EnemyAction lastEnemyAction = EnemyAction::Attack;
    int enemySkillCooldown = 0;

    bool playerPoisoned = false;
    int poisonTurns = 0;

    int playerCoins = 0;
    int selectedAction = 0;

    int skillCooldownTurns = 0;
    bool skillOnCooldown = false;

    std::string notificationText;
    int notificationTimer = 0;

    bool showAttackEffect = false;
    int attackEffectFrame = 0;

    bool isPlayerTurn = true;
    bool skipEnemyTurn = false;

    std::vector<NotificationObserver*> observers;
};

#endif // GAME_H


