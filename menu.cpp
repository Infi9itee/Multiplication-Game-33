#include "menu.h"
#include "utils.h"
#include <cstring>

// Displays the main menu using ncurses menu library and gets user choice
void showMainMenu(int &choice){
    const char *choices_arr[] = {
        "   New Game     ", "   Load Game    ",
        "   How to Play    ", "   Exit       "
    };
    int n_choices = sizeof(choices_arr) / sizeof(char *);
    int menu_width = 30, menu_height = n_choices + 4;
    int menu_y = (LINES - menu_height) / 2;
    int menu_x = (COLS - menu_width) / 2;
    WINDOW *menu_win = create_newwin(menu_height, menu_width, menu_y, menu_x); // graphical stuff starts here
    keypad(menu_win, TRUE);
    WINDOW *menu_sub_win = derwin(menu_win, n_choices, menu_width - 4, 2, 2);
    ITEM **items = new ITEM *[n_choices + 1];
    for(int i = 0; i < n_choices; i++){
        items[i] = new_item(choices_arr[i], "");
    }
    items[n_choices] = nullptr;
    MENU *menu = new_menu(items); // graphical stuff
    set_menu_win(menu, menu_win);
    set_menu_sub(menu, menu_sub_win);
    set_menu_mark(menu, " > ");
    set_menu_fore(menu, COLOR_PAIR(7) | A_REVERSE | A_BOLD);
    set_menu_back(menu, COLOR_PAIR(3));
    wattron(menu_win, A_BOLD | COLOR_PAIR(6));
    mvwprintw(menu_win, 1, (menu_width - 13) / 2, "- MAIN MENU -");
    wattroff(menu_win, A_BOLD | COLOR_PAIR(6));
    post_menu(menu);
    wrefresh(menu_win); // graphical stuff ends here
    int c;
    while(true){
        c = wgetch(menu_win);
        switch(c){
            case KEY_DOWN: case 's': case 'j': menu_driver(menu, REQ_DOWN_ITEM); break;
            case KEY_UP:   case 'w': case 'k': menu_driver(menu, REQ_UP_ITEM); break;
            case '\n': case KEY_ENTER: case ' ': goto item_selected;
        }
        wrefresh(menu_win);
    }

item_selected:
    choice = item_index(current_item(menu)); 
    unpost_menu(menu); wrefresh(menu_win); // graphical stuff starts here
    free_menu(menu);
    for(int i = 0; i < n_choices; i++){
        free_item(items[i]);
    }
    delete[] items;
    destroy_win(menu_sub_win);
    destroy_win(menu_win); // graphical stuff ends here
}

// Displays the game instructions in a window
void showInstructions(){
    int win_height = 16, win_width = 68;
    int win_y = (LINES - win_height) / 2;
    int win_x = (COLS - win_width) / 2;
    WINDOW *win = create_newwin(win_height, win_width, win_y, win_x); // graphical stuff starts here
    wattron(win, A_BOLD | COLOR_PAIR(6));
    mvwprintw(win, 1, (win_width - 15) / 2, "-- HOW TO PLAY --");
    wattroff(win, A_BOLD | COLOR_PAIR(6));
    wattron(win, COLOR_PAIR(7));
    mvwprintw(win, 3, 2, "1. The board contains numbers which are products of factors 1-9.");
    mvwprintw(win, 4, 2, "2. Players (Human [H] vs Computer [C]) take turns.");
    mvwprintw(win, 5, 2, "3. On your turn, choose a factor (1-9).");
    mvwprintw(win, 6, 2, "4. Multiply your chosen factor by the current 'Active Factor'.");
    mvwprintw(win, 7, 2, "5. Find the resulting product on the board and mark it [H] or [C].");
    mvwprintw(win, 8, 2, "   (You can only mark numbers that haven't been marked yet).");
    mvwprintw(win, 9, 2, "6. Your chosen factor becomes the new 'Active Factor' for the opponent.");
    mvwprintw(win, 10,2, "7. The first player to get 4 of their marks in a row (horizontally,");
    mvwprintw(win, 11,2, "   vertically, or diagonally) WINS!");
    wattroff(win, COLOR_PAIR(7));
    wattron(win, COLOR_PAIR(3));
    mvwprintw(win, win_height - 2, (win_width - 26) / 2, "Press any key to return...");
    wattroff(win, COLOR_PAIR(3));
    wrefresh(win);
    flushinp(); wgetch(win);
    destroy_win(win); // graphical stuff ends here
}