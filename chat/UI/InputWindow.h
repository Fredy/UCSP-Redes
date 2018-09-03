#include <iostream>
using namespace std;

#include "Window.h"

class InputWindow : public Window {
private:
public:
  InputWindow(int y, int x, int yinit, int xinit, string title)
      : Window(y, x, yinit, xinit, title) {}

  void init() {
    // readMessage();
    refresh();
    wrefresh(win);

    mvwaddstr(win, 1, 1, "ChatWindow");

    refresh();
    wrefresh(win);

    getch();
  }

  string readMessage() {
    std::string input;

    // let the terminal do the line editing
    nocbreak();
    echo();

    // this reads from buffer after <ENTER>, not "raw"
    // so any backspacing etc. has already been taken care of
    int ch = getch();

    while (ch != '\n') {
      input.push_back(ch);
      ch = getch();
    }

    // restore your cbreak / echo settings here

    return input;
  }
};