#include "common.hpp"
#include "ui.hpp"
#include <arpa/inet.h>
#include <ncurses.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unordered_map>

const int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
// ------ Declarations ------

enum class Operation {
  login = 'I',
  logout = 'O',

  sendMsg = 'M',
  list = 'L',
  broadcast = 'B',

  acknowledge = 'A',
  error = 'E',
}; 

unordered_map<string, int> clientsNames;
unordered_map<int, string> clientsIDS;

void login(int connectionID, string username);
void logout(int connectionID);

void sendMsg(string from, int sendTo, string message);
void broadcast(string from, string message);

void acknowledge(int connectionID, string code);
void error(int connectionID, string code);

void receive(int connectionID);
void waitForClients();

void listUsers(int connectionID);


int main(void) {
  // ------ Connection Setup ------
  sockaddr_in stSockAddr;

  if (-1 == SocketFD) {
    perror("can not create socket");
    exit(EXIT_FAILURE);
  }

  stSockAddr.sin_family = AF_INET;         // Socket type.
  stSockAddr.sin_port = htons(comm::PORT); // Setting the port.
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
  
  waitForClients();
  close(SocketFD);
}

void sendMsg(string from, int sendTo, string message) {
  // type, size_msg, msg, size_from, from
  char type = 'W'; // write
  char msgSize[3] = {0};
  char fromSize[3] = {0};

  snprintf(msgSize, 3, "%02d", message.size());
  snprintf(fromSize, 3, "%02d", from.size());


  // In this version just the first write error is checked
  int n = write(sendTo, &type, 1);

  if (n < 0) {
    cerr << "ERROR:PROTOCOL writing to socket\n";
  }

  write(sendTo, msgSize, 2);
  write(sendTo, message.c_str(), message.size());
  write(sendTo, fromSize, 2);
  write(sendTo, from.c_str(), from.size());

}

void waitForClients() {

  while (true) {
    int connectionID = accept(SocketFD, NULL, NULL);
    if (0 > connectionID) {
      cerr << "ERROR: Establish connection failed\n";
      continue;
    }

    cout << "NEW CLIENT: " << connectionID << '\n';
    thread(receive, connectionID).detach();

  }

}


void receive(int connectionID) {
  char operation;
  char buffer[comm::BUFFER_SIZE] = {0};
  while (true) {
    read(connectionID, &operation, 1);
    switch (Operation(operation)) {
    case Operation::login: {
      char usernameSize[3] = {0};
      char username[comm::BUFFER_SIZE] = {0};

      read(connectionID, usernameSize, 2);
      read(connectionID, username, stoi(usernameSize));

      login(connectionID, string(username, stoi(usernameSize)));
      break;
    }
    case Operation::logout: {
      logout(connectionID);
      break;
    }
    case Operation::sendMsg: {
      // type, size_msg, msg, size_to, to
      char msgSize[3] = {0};
      char sendToSize[3] = {0};
      char sendTo[comm::BUFFER_SIZE] = {0};

      read(connectionID, msgSize, 2);
      read(connectionID,buffer, stoi(msgSize));
      read(connectionID,sendToSize, 2);
      read(connectionID, sendTo, stoi(sendToSize));

      string sendToStr = string(sendTo, stoi(sendToSize));
      if (clientsNames.find(sendToStr) == clientsNames.end()) {
        error(connectionID, "20");
        break;
      }
      sendMsg(clientsIDS.at(connectionID),
              clientsNames.at(sendToStr),
              string(buffer, stoi(msgSize)));
      acknowledge(connectionID, "20");
      break;
    }
    case Operation::list: {
      listUsers(connectionID);
      break;
    }
    case Operation::broadcast: {
      // type, size_msg, msg, size_to, to
      char msgSize[3] = {0};

      read(connectionID, msgSize, 2);
      read(connectionID,buffer, stoi(msgSize));

      broadcast(clientsIDS.at(connectionID),
              string(buffer, stoi(msgSize)));
      acknowledge(connectionID, "20");
      break;
    }
    case Operation::acknowledge:
      break;
    case Operation::error:
      break;
    default: 
      read(connectionID, buffer, comm::BUFFER_SIZE);
      error(connectionID, "20");
      break;
    }
  }
}

void login(int connectionID, string username) {
  if (clientsIDS.find(connectionID) != clientsIDS.end() or clientsNames.find(username) != clientsNames.end()) {
    error(connectionID, "10");
    return;
  }
  clientsIDS[connectionID] = username;
  clientsNames[username] = connectionID;

  cout << "LOGIN: " << connectionID << " " << username << endl;
  acknowledge(connectionID, "10");
}

void logout(int connectionID) {
  if (clientsIDS.find(connectionID) == clientsIDS.end()) { 
    error(connectionID, "30");
    return;
  }

  clientsNames.erase(clientsIDS[connectionID]);
  clientsIDS.erase(connectionID);
  acknowledge(connectionID, "30");

  // shutdown(connectionID, SHUT_RDWR);
  // close(connectionID);
}

void listUsers(int connectionID) {
  char type = 'L';
  char numUsers[3] = {0};
  char usernameSize[3] = {0};

  snprintf(numUsers, 3, "%02d", clientsIDS.size());

  write(connectionID, &type, 1);
  write(connectionID, numUsers, 2);

  for (const auto& i : clientsNames) {
    snprintf(usernameSize, 3, "%02d", i.first.size());
    write(connectionID, usernameSize, 2);
    write(connectionID, i.first.c_str(), i.first.size());
  }
  
}


void acknowledge(int connectionID, string code) {
  char type = 'A';
  write(connectionID, &type, 1);
  write(connectionID, code.c_str(), 2);
}

void error(int connectionID, string code) {
  char type = 'E';
  write(connectionID, &type, 1);
  write(connectionID, code.c_str(), 2);
}

void broadcast(string from, string message) {
  for (const auto& i : clientsNames) {
    char sendToSize[3] = {0};
    char sendTo[comm::BUFFER_SIZE] = {0};


  if (i.first != from) {
    sendMsg(from, i.second, message);
  }
  }
}
