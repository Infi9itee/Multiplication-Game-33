#ifndef GAME_H
#define GAME_H

#include <ncurses.h>
#include <menu.h>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <string.h>
#include <limits>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
#include "menu.h"
#include <ncurses.h>
#include "constants.h"

const std::string SAVE_FILENAME = "multiplication_save.txt";
const int HUMAN_PLAYER = 1;
const int COMPUTER_PLAYER = 2;
const int NO_PLAYER = 0;
const int WIN_LENGTH = 4;
const int score_win = 10000;
const int thr_tw = 500;
const int thr_on = 100;
const int no_op = 50;

extern const int directions[4][2]; // Declare as extern for global access
extern int board[BOARD_SIZE][BOARD_SIZE];
extern int moveOwner[BOARD_SIZE][BOARD_SIZE];
struct BoardDisplayInfo;
struct GameState {
    int activeFactor;
    bool humanTurn;
};
void initializeGameState(GameState &state);
void playGame(GameState &state, bool loaded);
bool canPlayerMove(int currentActiveFactor);
bool checkLine(int start_r, int start_c, int dr, int dc, int player);
int checkWinCondition();
bool humanMove(GameState &state, const BoardDisplayInfo& displayInfo);
void computerMove(GameState &state, const BoardDisplayInfo& displayInfo);
void showGameOverMessage(int winner, bool userQuit = false);

#endif