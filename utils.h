#ifndef UTILS_H
#define UTILS_H

#include <ncurses.h>
#include <string>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>

struct GameState;

void showTempMessage(const std::string& message, int color_pair_attr, int height,
                     int desired_width, int duration_ms, int y_offset_from_bottom = 4);
bool saveGame(const GameState &state);
bool loadGame(GameState &state);
int computerChooseFactor(const GameState &state);
WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);
void resetGameMarkings();

#endif