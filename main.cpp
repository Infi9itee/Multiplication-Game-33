#include "game.h"
#include "menu.h"
#include "utils.h"
#include "board.h"
#include <ncurses.h>
#include <cstdlib>
#include <ctime>

int main() {
    srand(time(0));
    initializeBoard();
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);

    int menuChoice = -1;
    bool gameLoadedSuccessfully = false;

    do {
        gameLoadedSuccessfully = false;
        showMainMenu(menuChoice);

        GameState state;
        switch (menuChoice) {
            case 0: // New Game
                playGame(state, false);
                break;
            case 1: // Load Game
                if (loadGame(state)) {
                    showTempMessage("Game Loaded!", COLOR_PAIR(2) | A_BOLD, 3, 20, 1500);
                    gameLoadedSuccessfully = true;
                } else {
                    showTempMessage("Failed to load game or no save file found.", COLOR_PAIR(1) | A_BOLD, 3, 50, 2000);
                    clear();
                    refresh();
                }
                if (gameLoadedSuccessfully) playGame(state, true);
                break;
            case 2: // How to Play
                showInstructions();
                clear();
                refresh();
                break;
            case 3: // Exit
                break;
        }
    } while (menuChoice != 3);

    endwin();
    return 0;
}