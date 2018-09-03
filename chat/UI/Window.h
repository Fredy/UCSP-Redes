#ifndef _WINDOW_
#define _WINDOW_

#include <iostream>
#include <ncurses.h>
using namespace std;

#define MAXLINES 20
#define MAXCOLS 30

class Window {

public:
  Window(int y, int x, int start_y, int start_x, string title) {

    win = newwin(y, x, start_y, start_x);
    this->title = title;
    rows = y;
    cols = x;

    draw_borders();

    scrollok(win, TRUE);
    idlok(win, TRUE);
    /*
    refresh();
    wrefresh(win);
    //wprintw(win,"Hello, World.");
    mvwaddstr(win, 1, 1, "subwindow");
    //mvwprintw(win, 1, 1, "subwindow");
    refresh();
            wrefresh(win);

            getch();
            */
  }

  ~Window() = default;

  // protected:
  WINDOW *win;
  int rows, cols;

  std::string title;

  void draw_borders() {
    // cout << "dibujanfo borders"<< endl;
    wborder(win, 0, 0, 0, 0, 0, 0, 0, 0);
    box(win, 0, 0);
  }
};

#endif // _WINDOW_