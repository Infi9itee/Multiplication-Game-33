#ifndef BOARD_H
#define BOARD_H

#include "game.h"
#include <ncurses.h>
#include "constants.h"

extern int board[BOARD_SIZE][BOARD_SIZE];
extern int moveOwner[BOARD_SIZE][BOARD_SIZE];

struct BoardDisplayInfo {
    int start_y;
    int start_x;
    int cell_width;
    int total_width;
    int required_height;
    bool valid;
};

void initializeBoard();
bool isValidMove(int product);
bool markProduct(int product, int player, const BoardDisplayInfo& displayInfo);
bool wouldWin(int product, int player);
int evaluateMove(int product, int player);
BoardDisplayInfo getBoardDisplayInfo();
BoardDisplayInfo display_board_ncurses(const GameState &state);

#endif