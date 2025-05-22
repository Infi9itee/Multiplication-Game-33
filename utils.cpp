#include "utils.h"
#include "game.h"
#include "board.h"
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>
#include <cctype>

// Creates a new ncurses window with a border
WINDOW *create_newwin(int height, int width, int starty, int startx){
    if(starty < 0) starty = 0;
    if(startx < 0) startx = 0;
    if(height <= 0) height = 1;
    if(width <= 0) width = 1;
    if(starty + height > LINES) height = LINES - starty;
    if(startx + width > COLS) width = COLS - startx;
    WINDOW *local_win = newwin(height, width, starty, startx);
    box(local_win, 0, 0);
    wrefresh(local_win);
    return local_win;
}

// Destroys an ncurses window properly
void destroy_win(WINDOW *local_win){
    if(local_win){
        wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
        wrefresh(local_win);
        delwin(local_win);
    }
}

void showTempMessage(const std::string& message, int color_pair_attr, int height,
    int desired_width, int duration_ms, int y_offset_from_bottom){
    if(LINES <= height + y_offset_from_bottom || COLS <= desired_width){
       mvprintw(LINES - 1, 0, "Msg: %s", message.c_str());
       refresh();
       std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
       return;
    }
    int win_width = desired_width;
    int win_y = LINES - height - y_offset_from_bottom;
    int win_x = (COLS - win_width) / 2;
    WINDOW *msg_win = create_newwin(height, win_width, win_y, win_x);
    if(!msg_win) return;
    wattron(msg_win, color_pair_attr);
    int text_x = (win_width - message.length()) / 2;
    if(text_x < 1) text_x = 1;
    mvwprintw(msg_win, height / 2, text_x, "%s", message.c_str());
    wattroff(msg_win, color_pair_attr);
    wrefresh(msg_win);
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
    destroy_win(msg_win);
}

// Saves the current game state to a file
bool saveGame(const GameState &state){
    std::ofstream outFile(SAVE_FILENAME);
    if(!outFile.is_open()){
        std::string error = "Error: Could not open save file '" + SAVE_FILENAME + "' for writing!";
        showTempMessage(error, COLOR_PAIR(1) | A_BOLD, 3, error.length() + 4, 2500);
        return false;
    }
    // 1. Save GameState (activeFactor and whose turn it is)
    outFile << state.activeFactor << std::endl;
    outFile << (state.humanTurn ? 1 : 0) << std::endl; // Save boolean as 1 or 0
    // 2. Save the moveOwner board (which player owns which cell)
    for(int i = 0; i < BOARD_SIZE; ++i){
        for(int j = 0; j < BOARD_SIZE; ++j){
            outFile << moveOwner[i][j] << (j == BOARD_SIZE - 1 ? "" : " ");
        }
        outFile << std::endl;
    }
    outFile.close();
    return true;
}

// Reset the ownership of all board cells to NO_PLAYER
void resetGameMarkings() {
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            moveOwner[i][j] = NO_PLAYER;
        }
    }
}

bool loadGame(GameState &state){
    std::ifstream inFile(SAVE_FILENAME);
    if(!inFile.is_open()){
        return false; // Indicate failure (no save file found)
    }
    std::string line;
    int turnFlag;
    // 1. Load GameState
    if(!(inFile >> state.activeFactor)){ inFile.close(); return false; }
    if(!(inFile >> turnFlag)){ inFile.close(); return false; }
    state.humanTurn = (turnFlag == 1);
    inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    // 2. Load moveOwner board
    for(int i = 0; i < BOARD_SIZE; ++i){
        if(!std::getline(inFile, line)){ inFile.close(); return false; }
        std::stringstream ss(line);
        for(int j = 0; j < BOARD_SIZE; ++j){
            if(!(ss >> moveOwner[i][j])){ inFile.close(); return false; }
            if(moveOwner[i][j] != NO_PLAYER && moveOwner[i][j] != HUMAN_PLAYER 
                && moveOwner[i][j] != COMPUTER_PLAYER){
                inFile.close(); return false;
            }
        }
    }
    inFile.close();
    if(state.activeFactor < 1 || state.activeFactor > 9){
        resetGameMarkings();
        return false;
    }

    return true;
}

// AI logic to choose the best factor
int computerChooseFactor(const GameState &state){
    int bestFactor = -1;
    int maxScore = std::numeric_limits<int>::min();
    int blockingFactor = -1;
    std::vector<int> possibleFactors;
    for(int f = 1; f <= 9; f++){
        if(isValidMove(f * state.activeFactor)){
            possibleFactors.push_back(f);
        }
    }
    if(possibleFactors.empty()) return -1;
    // 1. Check for Computer Win
    for(int f : possibleFactors){
        if(wouldWin(f * state.activeFactor, COMPUTER_PLAYER)){
            return f;
        }
    }
    // 2. Check for Human Win Block
    for(int human_f = 1; human_f <= 9; human_f++){
        int human_product = human_f * state.activeFactor;
        if(isValidMove(human_product) && wouldWin(human_product, HUMAN_PLAYER)){
            for(int comp_f : possibleFactors){
                if(comp_f * state.activeFactor == human_product){
                    blockingFactor = comp_f;
                    break;
                }
            }
        }
        if(blockingFactor != -1) break;
    }
    if(blockingFactor != -1) return blockingFactor;
    for(int f : possibleFactors){
        int product = f * state.activeFactor;
        int currentScore = evaluateMove(product, COMPUTER_PLAYER);
        if(currentScore > maxScore){
            maxScore = currentScore;
            bestFactor = f;
        }
    }
    // random move if no best factor found
    if(bestFactor == -1 && !possibleFactors.empty()){
        bestFactor = possibleFactors[rand() % possibleFactors.size()];
    }

    return bestFactor;
}
