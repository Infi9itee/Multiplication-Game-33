#include "board.h"
#include "utils.h"
#include "game.h"
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <limits>
#include <string>

int board[BOARD_SIZE][BOARD_SIZE];
int moveOwner[BOARD_SIZE][BOARD_SIZE] = {0};

// Initialize Board with predefined non-prime numbers (and single-digit primes)
void initializeBoard(){
    const std::vector<int> nums = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14,
        15, 16, 18, 20, 21, 24, 25, 27, 28, 30, 32, 35,
        36, 40, 42, 45, 48, 49, 54, 56, 63, 64, 72, 81};
    int index = 0;
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            board[i][j] = nums[index++];
        }
    }
}

// Check if a product is available to occupy on the board
bool isValidMove(int product){
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            if(board[i][j] == product && moveOwner[i][j] == NO_PLAYER){
                return true;
            }
        }
    }
    return false;
}

// Temporarily marks a cell to see if it results in a win for the Computer
bool wouldWin(int product, int player){
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            if(board[i][j] == product && moveOwner[i][j] == NO_PLAYER){
                moveOwner[i][j] = player;
                bool wins = (checkWinCondition() == player);
                moveOwner[i][j] = NO_PLAYER;
                return wins;
            }
        }
    }
    return false;
}

// Marks the cell corresponding to the product for the given player
bool markProduct(int product, int player, const BoardDisplayInfo& displayInfo){
     for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            if(board[i][j] == product && moveOwner[i][j] == NO_PLAYER){
                moveOwner[i][j] = player;
                int color = (player == HUMAN_PLAYER) ? 1 : 4; // Red for Human, Blue for Computer
                int current_row_y = displayInfo.start_y + 1 + i * 2;
                int cell_start_x = displayInfo.start_x + 1 + j * displayInfo.cell_width;
                attron(A_BOLD | COLOR_PAIR(color)); // Graphical stuff
                mvprintw(current_row_y, cell_start_x + 1, "[%c%*d]",
                         (player == HUMAN_PLAYER ? 'H' : 'C'), displayInfo.cell_width - 4, board[i][j]);
                attroff(A_BOLD | COLOR_PAIR(color));
                refresh();
                return true;
            }
        }
    }
    return false;
}

// Simple evaluation for a potential computer move
int evaluateMove(int product, int player){
    int score = 0;
    int r = -1, c = -1;
    for(int i = 0; i < BOARD_SIZE && r == -1; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            if(board[i][j] == product && moveOwner[i][j] == NO_PLAYER){
                r = i; c = j; break;
            }
        }
    }
    if(r == -1) return std::numeric_limits<int>::min();
    moveOwner[r][c] = player;
    for(auto &dir : directions){
        int consecutive = 0;
        int open_ends = 0;
        // Check positive direction
        for(int k = 1; k < 4; ++k){
            int nr = r + k * dir[0]; int nc = c + k * dir[1];
            if(nr < 0 || nr >= BOARD_SIZE || nc < 0 || nc >= BOARD_SIZE) break;
            if(moveOwner[nr][nc] == player) consecutive++;
            else if(moveOwner[nr][nc] == NO_PLAYER) { open_ends++; break; }
            else break;
        }
        // Check negative direction
         for(int k = 1; k < 4; ++k){
            int nr = r - k * dir[0]; int nc = c - k * dir[1];
            if(nr < 0 || nr >= BOARD_SIZE || nc < 0 || nc >= BOARD_SIZE) break;
            if(moveOwner[nr][nc] == player) consecutive++;
            else if(moveOwner[nr][nc] == NO_PLAYER) { open_ends++; break; }
            else break;
        }
        consecutive++;
        // Scoring
        if(consecutive >= 4) score += score_win;
        else if(consecutive == 3 && open_ends >= 1) score += (open_ends == 2 ? thr_tw : thr_on);
        else if(consecutive == 2 && open_ends == 2) score += no_op;
    }
    int center_start = BOARD_SIZE / 2 - 1;
    int center_end = BOARD_SIZE / 2;
    if(r >= center_start && r <= center_end && c >= center_start && c <= center_end){
        score += 2;
    }
    moveOwner[r][c] = NO_PLAYER;
    return score;
}

// Calculates the board display dimensions and position
BoardDisplayInfo getBoardDisplayInfo() {
    BoardDisplayInfo info;
    info.cell_width = 7;
    info.total_width = BOARD_SIZE * info.cell_width + 1;
    info.required_height = 2 * BOARD_SIZE + 1;
    info.start_y = 6;
    info.start_x = (COLS - info.total_width) / 2;
    if(info.start_x < 0) info.start_x = 0;
    info.valid = true;
    if(info.start_y + info.required_height >= LINES || info.total_width >= COLS){
        info.valid = false;
    }
    return info;
}

// Displays the main game board and current game state (Graphical stuff)
BoardDisplayInfo display_board_ncurses(const GameState &state){
    clear();
    BoardDisplayInfo displayInfo = getBoardDisplayInfo(); // graphical stuff
    attron(A_BOLD | COLOR_PAIR(6));
    mvprintw(1, (COLS - 20)/2, "MULTIPLICATION GAME"); 
    attroff(A_BOLD | COLOR_PAIR(6));
    attron(COLOR_PAIR(3));
    mvprintw(3, (COLS-25)/2, "    Active Factor: %d", state.activeFactor);
    attroff(COLOR_PAIR(3));
    attron(A_BOLD | COLOR_PAIR(state.humanTurn ? 1 : 4));
    mvprintw(4, (COLS-20)/2, "  %s's turn", state.humanTurn ? "Human" : "Computer");
    attroff(A_BOLD | COLOR_PAIR(state.humanTurn ? 1 : 4));
    if(!displayInfo.valid){
        attron(COLOR_PAIR(1)|A_BOLD); // graphical stuff
        mvprintw(LINES / 2, (COLS - 30) / 2, "Terminal too small to draw board!");
        attroff(COLOR_PAIR(1)|A_BOLD);
        refresh();
        return displayInfo;
    }
    mvaddch(displayInfo.start_y, displayInfo.start_x, ACS_ULCORNER);
    for(int j = 0; j < BOARD_SIZE; j++){
        mvhline(displayInfo.start_y, displayInfo.start_x + 1 + j *  // graphical stuff
            displayInfo.cell_width, ACS_HLINE, displayInfo.cell_width - 1);
        mvaddch(displayInfo.start_y, displayInfo.start_x + displayInfo.cell_width + j * 
            displayInfo.cell_width, (j < BOARD_SIZE - 1) ? ACS_TTEE : ACS_URCORNER);
    }
    for(int i = 0; i < BOARD_SIZE; i++){
        int current_row_y = displayInfo.start_y + 1 + i * 2;
        int separator_row_y = displayInfo.start_y + 2 + i * 2;
        mvaddch(current_row_y, displayInfo.start_x, ACS_VLINE);
        for(int j = 0; j < BOARD_SIZE; j++){
            int cell_start_x = displayInfo.start_x + 1 + j * displayInfo.cell_width;
            if(moveOwner[i][j] != NO_PLAYER){
                int color_pair = (moveOwner[i][j] == HUMAN_PLAYER) ? 1 : 4;
                attron(A_BOLD | COLOR_PAIR(color_pair)); // graphical stuff
                mvprintw(current_row_y, cell_start_x + 1, "[%c%*d]",
                        (moveOwner[i][j] == HUMAN_PLAYER) ? 'H' : 'C',
                        displayInfo.cell_width - 4,
                        board[i][j]);
                attroff(A_BOLD | COLOR_PAIR(color_pair));
            }else{
                mvprintw(current_row_y, cell_start_x, " %*d ", displayInfo.cell_width - 2, board[i][j]);
            }
            mvaddch(current_row_y, displayInfo.start_x + displayInfo.cell_width + j * 
                displayInfo.cell_width, ACS_VLINE);
        }
        if(i < BOARD_SIZE - 1){
            mvaddch(separator_row_y, displayInfo.start_x, ACS_LTEE); // graphical stuff
            for (int j = 0; j < BOARD_SIZE; j++){
                mvhline(separator_row_y, displayInfo.start_x + 1 + j * 
                    displayInfo.cell_width, ACS_HLINE, displayInfo.cell_width - 1);
                mvaddch(separator_row_y, displayInfo.start_x + displayInfo.cell_width + j * 
                    displayInfo.cell_width, (j < BOARD_SIZE - 1) ? ACS_PLUS : ACS_RTEE);
            }
        }
    }
    int bottom_border_y = displayInfo.start_y + 2 * BOARD_SIZE;
    mvaddch(bottom_border_y, displayInfo.start_x, ACS_LLCORNER); // graphical stuff
    for(int j = 0; j < BOARD_SIZE; j++){
        mvhline(bottom_border_y, displayInfo.start_x + 1 + j *  
            displayInfo.cell_width, ACS_HLINE, displayInfo.cell_width - 1);
        mvaddch(bottom_border_y, displayInfo.start_x + displayInfo.cell_width + j * 
            displayInfo.cell_width, (j < BOARD_SIZE - 1) ? ACS_BTEE : ACS_LRCORNER);
    }
    refresh();
    return displayInfo;
}