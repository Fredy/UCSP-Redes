#include "ChatWindow.h"
#include "InputWindow.h"
#include <iostream>
#include <thread>
using namespace std;

int main(int argc, char const *argv[]) {
  initscr();

  // Window* win = new Window(20,50,5,5,"ventana nueva");
  ChatWindow chatwin(20, 50, 5, 5, "chat");
  InputWindow inputwin(20, 10, 51, 5, "input");

  // HELP ME NIGGAA !!!!!!
  // thread* thinput= new thread(InputWindow::init);

  chatwin.createThread();
  // thinput->detach();
  endwin();
  return 0;
}
