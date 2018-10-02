/**
 * @brief definition of server class
 * @file server.cpp
 */

#include <unordered_map>
#include <cstdlib>
#include <map>
#include <vector>
#include <queue>
#include <iostream>
#include <arpa/inet.h>
#include <ncurses.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <mutex>
#include <string>
#include <unistd.h>
#include <cstring>
#include <random>

using namespace std;

/**
 * @brief Message Types contains the different message types
 * in the protocol 
 */
enum class Message_Types {
    login = 'I',
    logout = 'O',
    send_msg = 'M',
    userlist = 'L',
    broadcast = 'U',
    ack = 'A',
    error = 'E'
};

const int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

std::mutex mtx_login;
std::mutex mtx_game;

 std::map<int, std::__cxx11::string> ids;
 std::map<std::__cxx11::string, int> names;
 std::vector<std::vector<char> > matrix(20, vector<char>(20, '.'));
 int cols = 20;
 int rows = 20;
 void send_message(char mtype, int idto, std::__cxx11::string message){
    write(idto, &mtype, 1);
    std::__cxx11::string scols = std::to_string(cols);
    std::__cxx11::string srows = std::to_string(rows);
    write(idto, scols.c_str(), 2);
    write(idto, srows.c_str(), 2);
    write(idto, message.c_str(), cols*rows);
 }

 void broadcast(){
    std::__cxx11::string msg = "";
    for(auto v:matrix){
       for(auto i:v){
          msg += i;
       }
    }
    for(const auto& i: ids){
       send_message('M',i.first,msg);
    }
 }

 void macknowledge(int conn_id, std::__cxx11::string code){
    char mtype = 'A';
    write(conn_id, &mtype, 1);
    write(conn_id, code.c_str(), 2);
 }

 void merror(int conn_id, std::__cxx11::string code){
    char mtype = 'E';
    write(conn_id, &mtype, 1);
    write(conn_id, code.c_str(), 2);
 }

 bool insert_to_map(std::__cxx11::string username, int connection_id){
    mtx_login.lock();
    if(names.find(username) != names.end() or ids.find(connection_id) != ids.end()){
       // USERNAME EXISTS
       mtx_login.unlock();
       return false;
    } 
    // USERNAME is new, insert to "database"(map :v)
    names[username] = connection_id;
    ids[connection_id] = username;
    mtx_login.unlock();
    return true;
 }

 bool delete_from_map(std::__cxx11::string username){
    mtx_login.lock();
    if(names.find(username) == names.end()){
       // USERNAME DOESNT EXISTS
       mtx_login.unlock();
       return false;
    }
    ids.erase(names[username]);
    names.erase(username);
    mtx_login.unlock();
    return true;
 }

 bool delete_from_map(int conn_id){
    mtx_login.lock();
    if(ids.find(conn_id) == ids.end()){
       // USERNAME DOESNT EXISTS
       mtx_login.unlock();
       return false;
    }
    names.erase(ids[conn_id]);
    ids.erase(conn_id);
    mtx_login.unlock();
    return true;
 }

bool validate(int posx, int posy){
   if(posx < 0 or posx >=20) return false;
   if(posy < 0 or posy >=20) return false;
   return true;
}

bool playGame(int conn_id, int row, int col){
   char player = ids[conn_id][0];
   if(!validate(row, col))return true;
   for(int i=0;i<rows;++i){
      for(int j=0;j<cols;++j){
         if(matrix[i][j] == player){
            if(matrix[row][col] == '&'){
               matrix[i][j] = '.';
               return false;
            }
            else if(matrix[row][col] != '.'){
            }
            else if(matrix[row][col] == '.'){
               matrix[i][j] = '.';
               matrix[row][col] = player;
            }
            break;
         }
      }
   }
   return true;
}

 void receive(int conn_id){
     char operation;
     while(true){
         read(conn_id, &operation, 1);
         switch (Message_Types(operation)){
            case Message_Types::login:{
               // LOGIN (just 1 char)
               char username[2] = {0};
               read(conn_id, username, 1);
               std::__cxx11::string username_str = std::__cxx11::string(username);
               if(insert_to_map(username_str, conn_id)){
                  macknowledge(conn_id, "10");
                  mtx_game.lock();
                  for(int i=0;i<rows;++i){
                     for(int j=0;j<cols;++j){
                        if(matrix[i][j] == '.'){
                           matrix[i][j] = username_str[0];
                           break;
                        }
                     }
                  }
                  mtx_game.unlock();
               } else {
                  merror(conn_id, "10");
               }
               break;
            }
            case Message_Types::logout:{
               // LOGOUT
               char nn = ids[conn_id][0];
               if(delete_from_map(conn_id)){
                  mtx_game.lock();
                  for(int i=0;i<rows;++i){
                     for(int j=0;j<cols;++j){
                        if(matrix[i][j] == nn){
                           matrix[i][j] = '.';
                           break;
                        }
                     }
                  }
                  mtx_game.unlock();
                  macknowledge(conn_id, "20");
               } else {
                  merror(conn_id, "20");
               }
               break;
            }
            case Message_Types::send_msg:{
               // Message received from client
               mtx_game.lock();
               char poscol[3] = {0};
               read(conn_id, poscol, 2);
               int col = atoi(poscol);
               char posrow[3] = {0};
               read(conn_id, posrow, 2);
               int row = atoi(posrow);
               if(!playGame(conn_id,row, col)){
                  if(delete_from_map(conn_id)){
                     macknowledge(conn_id, "40");
                  } else {
                     merror(conn_id, "40");
                  }
               }else {
                  broadcast();
               }
               mtx_game.unlock();
               break;
            }

            case Message_Types::broadcast:{
               // Message received from client
               mtx_game.lock();
               broadcast();
               mtx_game.unlock();
               break;
            }
            default:{
               merror(conn_id,"20");
               std::cout << "error in message, type is not valid" << std::endl;
            }
         }
     }
 }

void move_snake(int px, int py,std::deque<pair<int, int> > &vec){
   if(!validate(px, py))return;
   if(matrix[py][px]=='&')return;
   if(matrix[px][py]!='.'){
      string name = "";name+=matrix[py][px];
      int conn_id = names[name];
               if(delete_from_map(conn_id)){
                  macknowledge(conn_id, "40");
               } else {
                  merror(conn_id, "40");
               }
   }
   matrix[py][px] = '&';
   matrix[vec[vec.size()-1].first][vec[vec.size()-1].second] = '.';
   vec.push_front({py,px});
   vec.pop_back();
}

void snake_playing(){
   std::deque<pair<int,int> > snake;
   snake.push_back({10,10});
   snake.push_back({10,11});
   snake.push_back({10,12});
   snake.push_back({10,13});
   matrix[10][10] = '&';
   matrix[10][11] = '&';
   matrix[10][12] = '&';
   matrix[10][13] = '&';
   std::random_device rd;
   std::mt19937 mt(rd());
   std::uniform_int_distribution<int> dist(1, 4);
   while(true){
      usleep(1000000);
      int dir = dist(mt);

      int px,py;
      if(dir == 1){
         px = snake[0].second+1; py = snake[0].first;
      }else if(dir==2){
         px = snake[0].second; py = snake[0].first-1;
      }else if(dir==3){
         px = snake[0].second; py = snake[0].first+1;
      }else{
         px = snake[0].second-1; py = snake[0].first;
      }
      move_snake(px,py,snake);
   }
}

void print_state(){
   while(true){
      std::system("clear");
   for(auto v: matrix){
      for(auto w:v){
         cout << w << ' ';
      }
      cout << endl;
   }
   }
}

 void wait_for_clients(void){
    auto t1 = std::thread(print_state);
    std::thread(snake_playing).detach();
    while(true){
       int connectionid = accept(SocketFD, NULL, NULL);
       if(0 > connectionid){
          std::cout << "error creating connection" << std::endl;
          continue;
       }
       std::cout << "new client id: " << connectionid << std::endl;
       std::thread(receive, connectionid).detach();
    }
    t1.join();
 }


int main(){
// ------ Connection Setup ------
  sockaddr_in stSockAddr;

  if (-1 == SocketFD) {
    perror("can not create socket");
    exit(EXIT_FAILURE);
  }

  stSockAddr.sin_family = AF_INET;         // Socket type.
  stSockAddr.sin_port = htons(8888); // Setting the port.
  stSockAddr.sin_addr.s_addr = INADDR_ANY; // The address could be anything.

  // Bind the structure with the socket using bind()
  if (-1 ==
      bind(SocketFD, (const sockaddr *)&stSockAddr, sizeof(sockaddr_in))) {
    perror("error bind failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  // Setting the server to listening mode so it can listen conections.
  if (-1 == listen(SocketFD, 10)) {
    perror("error listen failed");
    close(SocketFD);
    exit(EXIT_FAILURE);
  }
  wait_for_clients();
  close(SocketFD);

}
