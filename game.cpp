#include "game.h"
#include "board.h"
#include "utils.h"
#include <cstdlib>
#include <ctime>
#include <string>
#include <thread>
#include <chrono>
const int directions[4][2] = {
    {-1, 0},  // Up
    {1, 0},   // Down
    {0, -1},  // Left
    {0, 1}    // Right
};


// Check if the current player has at least one valid move available
bool canPlayerMove(int currentActiveFactor){
     for(int factor = 1; factor <= 9; factor++){
        if(isValidMove(factor * currentActiveFactor)){
            return true;
        }
    }
    return false;
}

void initializeGameState(GameState &state) {
    state.activeFactor = 1 + rand() % 9;
    state.humanTurn = (rand() % 2 == 0);
}

bool checkLine(int start_r, int start_c, int dr, int dc, int player){
    for(int k = 1; k < WIN_LENGTH; k++){
        int r = start_r + k * dr;
        int c = start_c + k * dc;
        if(r < 0 || r >= BOARD_SIZE || c < 0 || c >= BOARD_SIZE || moveOwner[r][c] != player){
            return false;
        }
    }
    return true;
}

// Check if a player has won (4 in a row horizontally, vertically, or diagonally)
int checkWinCondition(){
    for(int i = 0; i < BOARD_SIZE; i++){
        for(int j = 0; j < BOARD_SIZE; j++){
            int owner = moveOwner[i][j];
            if(owner == NO_PLAYER) continue;
            for(auto& dir: directions){
                int end_r = i + (WIN_LENGTH - 1) * dir[0];
                int end_c = j + (WIN_LENGTH - 1) * dir[1];
                if(end_r >= 0 && end_r < BOARD_SIZE && end_c >= 0 && end_c < BOARD_SIZE){
                    if(checkLine(i, j, dir[0], dir[1], owner)){
                        return owner;
                    }
                }
            }
        }
    }
    return NO_PLAYER;
}

bool humanMove(GameState &state, const BoardDisplayInfo& displayInfo){
    int input_win_height = 7, input_win_width = 60;
    int input_win_y = LINES - input_win_height - 1;
    int input_win_x = (COLS - input_win_width) / 2;
    WINDOW *input_win = create_newwin(input_win_height, input_win_width, input_win_y, input_win_x);
    keypad(input_win, TRUE);
    int factor = -1;
    std::string error_msg = "";
    std::string input_str = "";

    while(true){
        werase(input_win); 
        box(input_win, 0, 0);

        // Display instructions
        wattron(input_win, COLOR_PAIR(3));
        mvwprintw(input_win, 1, 2, "Active Factor: %d", state.activeFactor);
        mvwprintw(input_win, 2, 2, "Press any number to move, 'q' to exit, or 's' to save.");
        wattroff(input_win, COLOR_PAIR(3));

        // Display error message (if any)
        if(!error_msg.empty()){
            wattron(input_win, COLOR_PAIR(1) | A_BOLD);
            mvwprintw(input_win, 3, 2, "Error: %s", error_msg.c_str());
            wattroff(input_win, COLOR_PAIR(1) | A_BOLD);
            error_msg = "";
        }

        // Prompt for input
        wattron(input_win, COLOR_PAIR(7));
        mvwprintw(input_win, 5, 2, "Enter factor, 'q', or 's': ");
        wattroff(input_win, COLOR_PAIR(7));

        // Refresh the input window
        wrefresh(input_win);

        // Clear the input string and enable input mode
        input_str = "";
        echo(); 
        curs_set(1); 

        // Move the cursor to the position after the colon
        wmove(input_win, 5, 30);

        // Read input character by character
        int ch;
        while((ch = wgetch(input_win)) != '\n' && ch != KEY_ENTER){
            if(ch == 'q' || ch == 'Q'){
                destroy_win(input_win); 
                curs_set(0); 
                noecho();
                return false; // Exit the game
            }
            if(ch == 's' || ch == 'S'){
                // Save the game
                if(saveGame(state)){
                    showTempMessage("Game Saved!", COLOR_PAIR(2) | A_BOLD, 3, 20, 1500);
                } else {
                    showTempMessage("Save Failed!", COLOR_PAIR(1) | A_BOLD, 3, 20, 1500);
                }

                // Refresh the input window after saving
                flushinp();
                werase(input_win); 
                box(input_win, 0, 0);

                // Redraw the instructions and refresh the window
                wattron(input_win, COLOR_PAIR(3));
                mvwprintw(input_win, 1, 2, "Active Factor: %d", state.activeFactor);
                mvwprintw(input_win, 2, 2, "Press any number to move, 'q' to exit, or 's' to save.");
                wattroff(input_win, COLOR_PAIR(3));

                wattron(input_win, COLOR_PAIR(7));
                mvwprintw(input_win, 5, 2, "Enter factor, 'q', or 's': ");
                wattroff(input_win, COLOR_PAIR(7));

                wrefresh(input_win);

                // Reset the cursor position after the colon
                wmove(input_win, 5, 30);

                continue; // Stay in the loop to allow further input
            }

            // Handle backspace
            if(ch == KEY_BACKSPACE || ch == 127){
                if(!input_str.empty()){
                    input_str.pop_back();
                    int current_x, current_y;
                    getyx(input_win, current_y, current_x);
                    mvwdelch(input_win, current_y, current_x - 1);
                    wrefresh(input_win);
                }

                // Ensure the cursor stays after the colon
                wmove(input_win, 5, 30);
                continue;
            }

            // Only allow digits to be entered
            if(isdigit(ch)){
                input_str += (char)ch;
            }

            // Reset the cursor position after each input
            wmove(input_win, 5, 30 + input_str.length());
        }

        // Disable input mode and hide the cursor
        noecho(); 
        curs_set(0);

        // Validate the input
        if(input_str.empty()){
            error_msg = "No factor entered.";
            continue;
        }

        try{
            factor = std::stoi(input_str);
        }catch(const std::invalid_argument& ia) {
            error_msg = "Invalid number format.";
            continue;
        }catch(const std::out_of_range& oor) {
            error_msg = "Number out of range.";
            continue;
        }

        if(factor < 1 || factor > 9){
            error_msg = "Invalid factor! Must be between 1 and 9.";
            continue;
        }else{
            int product = factor * state.activeFactor;
            if(!isValidMove(product)){
                error_msg = "Product " + std::to_string(product) + " (" +
                            std::to_string(factor) + "*" + std::to_string(state.activeFactor) +
                            ") is not available!";
                continue;
            }else{
                if(markProduct(product, HUMAN_PLAYER, displayInfo)){
                    state.activeFactor = factor;
                    destroy_win(input_win);
                    return true;
                }else{
                    error_msg = "Internal error: Failed to mark valid product.";
                    destroy_win(input_win); 
                    return true;
                }
            }
        }
    }
}

// Handles the computer player's move
void computerMove(GameState &state, const BoardDisplayInfo& displayInfo){
    showTempMessage("Computer is thinking...", COLOR_PAIR(4) | A_BOLD, 3, 30, 800 + rand() % 700);
    // Get AI move
    int factor = computerChooseFactor(state);
    if(factor == -1){
        showTempMessage("No valid moves - Computer passes", COLOR_PAIR(3), 3, 40, 1500);
    }else{
        int product = factor * state.activeFactor;
        std::string comp_choice_msg = "Computer chose factor " + std::to_string(factor) + ", marking " + std::to_string(product);
        showTempMessage(comp_choice_msg, COLOR_PAIR(4), 3, comp_choice_msg.length() + 4, 1000);
        if(markProduct(product, COMPUTER_PLAYER, displayInfo)){
            state.activeFactor = factor;
        }
    }
}

// Displays the win/draw/quit message
void showGameOverMessage(int winner, bool userQuit){
    std::string message, detail;
    int color_pair;
    if(userQuit){
        message = ">>> GAME EXITED <<<"; detail = "(Returned to Main Menu)"; color_pair = 3;
    }else if(winner == HUMAN_PLAYER){
        message = ">>> HUMAN WINS! <<<"; detail = "(You got 4 in a row)"; color_pair = 2;
    }else if(winner == COMPUTER_PLAYER){
        message = ">>> COMPUTER WINS! <<<"; detail = "(Computer got 4 in a row)"; color_pair = 1;
    }else{
        message = ">>> DRAW! <<<"; detail = "(Neither player can move)"; color_pair = 3;
    }
    int msg_len = message.length();
    int det_len = detail.length();
    int prompt_len = 26;
    int win_width = std::max({msg_len, det_len, prompt_len}) + 6;
    int win_height = 7;
    int win_y = (LINES - win_height) / 2;
    int win_x = (COLS - win_width) / 2;
    WINDOW *win = create_newwin(win_height, win_width, win_y, win_x);
    wclear(win); box(win, 0, 0); // Grapgical stuff
    wattron(win, A_BOLD | COLOR_PAIR(color_pair));
    mvwprintw(win, 2, (win_width - msg_len) / 2, "%s", message.c_str());
    wattroff(win, A_BOLD | COLOR_PAIR(color_pair));
    wattron(win, COLOR_PAIR(7));
    mvwprintw(win, 3, (win_width - det_len) / 2, "%s", detail.c_str());
    wattroff(win, COLOR_PAIR(7));
    wattron(win, COLOR_PAIR(3));
    mvwprintw(win, 5, (win_width - prompt_len) / 2, "Press any key to continue");
    wattroff(win, COLOR_PAIR(3));
    wrefresh(win);
    flushinp(); wgetch(win);
    destroy_win(win);
}

void playGame(GameState &state, bool loaded) {
    if (!loaded) {
        resetGameMarkings();
        initializeGameState(state);
    }

    int winner = NO_PLAYER;
    bool userQuit = false;
    BoardDisplayInfo displayInfo;

    while (winner == NO_PLAYER) {
        displayInfo = display_board_ncurses(state);

        if (state.humanTurn) {
            int prompt_h = 3, prompt_w = 55;
            int prompt_y = LINES - prompt_h - 1, prompt_x = (COLS - prompt_w) / 2;
            WINDOW *prompt_win = create_newwin(prompt_h, prompt_w, prompt_y, prompt_x);
            wattron(prompt_win, COLOR_PAIR(3));
            mvwprintw(prompt_win, 1, 2, "Your turn. Press 's' to SAVE, any other key to MOVE.");
            wattroff(prompt_win, COLOR_PAIR(3));
            wrefresh(prompt_win);

            nodelay(stdscr, TRUE);
            timeout(2500);
            int ch = getch();
            timeout(-1);
            nodelay(stdscr, FALSE);

            destroy_win(prompt_win);
            flushinp();

            if (ch == 's' || ch == 'S') {
                if (saveGame(state)) {
                    showTempMessage("Game Saved!", COLOR_PAIR(2) | A_BOLD, 3, 20, 1500);
                } else {
                    showTempMessage("Save Failed!", COLOR_PAIR(1) | A_BOLD, 3, 20, 1500);
                }
                displayInfo = display_board_ncurses(state);
                if (!displayInfo.valid) return;
            }
        }

        bool currentPlayerCanMove = canPlayerMove(state.activeFactor);
        if (!currentPlayerCanMove) {
            std::string pass = (state.humanTurn ? "Human" : "Computer");
            pass += " has no valid moves. Passing turn.";
            showTempMessage(pass, COLOR_PAIR(3), 3, pass.length() + 4, 2000);
            state.humanTurn = !state.humanTurn;
            bool opponentCanMove = canPlayerMove(state.activeFactor);
            if (!opponentCanMove) {
                winner = 3;
                break;
            } else continue;
        }

        if (state.humanTurn) {
            if (!humanMove(state, displayInfo)) {
                userQuit = true;
                break;
            }
        } else {
            computerMove(state, displayInfo);
        }

        winner = checkWinCondition();
        if (winner == NO_PLAYER) {
            state.humanTurn = !state.humanTurn;
        }
    }

    if (winner != 3 && !userQuit) {
        display_board_ncurses(state);
    }
    showGameOverMessage(winner, userQuit);
    clear();
    refresh();
}