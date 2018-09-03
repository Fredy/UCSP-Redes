#include "InputWindow.h"
#include "ChatWindow.h"
#include <thread>
using namespace std;

int main(int argc, char const *argv[])
{
	initscr();

	//Window* win = new Window(20,50,5,5,"ventana nueva");
	ChatWindow* chatwin = new ChatWindow(20,50,5,5,"chat");
	InputWindow* inputwin = new InputWindow(20,10,51,5,"input");

	// HELP ME NIGGAA !!!!!! 
	//thread* thchat= new thread(&ChatWindow::init);
	//thread* thinput= new thread(InputWindow::init);

	thchat->detach();
	//thinput->detach();

	endwin();
	return 0;
}
