#include "Game.h"
#include "LevelData.h"
#include <iostream>

extern std::vector<LevelData> loadLevels();

#ifdef _WIN32
#include <windows.h>
void clearScreen() { system("cls"); }
#else
void clearScreen() { system("clear"); }
#endif

Game::Game()
    : currentLevel(0), map(1, 1) 
{
    levels = loadLevels();
    map.loadLevel(levels[0]);
    player = map.getPlayer();
}

void Game::run() {
    char input;

    while (true) {
        clearScreen();
        map.render();

        if (!player->isAlive()) {
            std::cout << "You died!\n";
            break;
        }

        if (map.isExitReached()) {
            ++currentLevel;
            if (currentLevel >= levels.size()) {
                std::cout << "🎉 You completed all levels! Victory!\n";
                break;
            }
            std::cout << "Level completed! Moving to next...\n";
            std::cin.ignore();
            map.loadLevel(levels[currentLevel], player);
            continue;
        }

        if (currentLevel == levels.size() - 1 && map.areAllEnemiesDefeated()) {
            std::cout << "🏆 You defeated all enemies in the final level!\n";
            std::cout << "🎉 You win the game!\n";
            break;
        }


        std::cout << "Move (w/a/s/d), quit (q): ";
        std::cin >> input;

        int dx = 0, dy = 0;
        if (input == 'q') break;
        if (input == 'w') dy = -1;
        else if (input == 's') dy = 1;
        else if (input == 'a') dx = -1;
        else if (input == 'd') dx = 1;

        map.movePlayer(dx, dy);
        map.updateEnemies();
    }
}
