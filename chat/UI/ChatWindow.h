#include <iostream>
#include <string>
#include <thread>
using namespace std;

#include "Window.h"
class ChatWindow : public Window {

private:
  int lastRow;

public:
  ChatWindow(int y, int x, int yinit, int xinit, string title)
      : Window(y, x, yinit, xinit, title) {
    lastRow = 0;
  }

  void init() {
    refresh();
    wrefresh(win);

    mvwaddstr(win, 1, 1, "ChatWindow");

    refresh();
    wrefresh(win);

    getch();
  }

  void printMessage(string message) {
    mvwaddstr(win, lastRow + 1, 1, message.c_str());
  }

  void createThread() {
    thread thr = thread(&ChatWindow::init, this);
    thr.detach();
  }

private:
  void draw(int num_lines, int start);
};
