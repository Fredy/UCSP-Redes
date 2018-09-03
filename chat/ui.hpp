#pragma once

#include <chrono>
#include <ncurses.h>
#include <string>
#include <thread>
using namespace std;

class NcursesUI {
private:
  WINDOW *outputWin, *inputWin;
  int line;

void clearWindow(WINDOW *window) {
    wclear(window);
    box(window, 0, 0);
    wrefresh(window);
  }

  string read(WINDOW *window) {
    clearWindow(window);
    mvwprintw(window, 1, 1, ">> ");
    char str[256];
    wgetstr(window, str);
    return str;
  }

  void write(WINDOW *window, string message) {
    if (line >= LINES - 4) {
      clearWindow(window);
      line = 1;
    }
    mvwprintw(window, line, 1, message.c_str());
    wrefresh(window);
    line++;
  }

  WINDOW *createWindow(int height, int width, int starty, int startx) {
    WINDOW *local_win;

    local_win = newwin(height, width, starty, startx);
    box(local_win, 0, 0);
    wrefresh(local_win);

    return local_win;
  }
public:

  void init() {
    line = 1;

    initscr(); // Start curses mode.
    refresh();

    outputWin = createWindow(LINES - 3, COLS, 0, 0);
    inputWin = createWindow(3, COLS, LINES - 3, 0);
  }

  void terminate() {
    endwin(); // End curses mode.
  }

  string readInput() {
    return read(inputWin);
  }

  void writeOutput(string str) {
    write(outputWin, str);
  }
  
};