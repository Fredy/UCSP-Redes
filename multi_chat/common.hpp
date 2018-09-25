#pragma once

#include "ui.hpp"
#include <atomic>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>
using namespace std;

namespace comm {
const int PORT = 2924;
const int BUFFER_SIZE = 256;
const int PROTOCOL_BUFFER_SIZE = 5;
const string QUIT_COMMAND = "quit";

string readWithProtocol(const int connectionID) {
  char type;
  char buffer[BUFFER_SIZE] = {0};

  int n = read(connectionID, &type, 1);

  if (type == 'W') {
    // type, size_msg, msg, size_from, from
    char msgSize[3] = {0};
    char fromSize[3] = {0};
    char from[BUFFER_SIZE] = {0};

    read(connectionID, msgSize, 2);
    read(connectionID, buffer, stoi(msgSize));
    read(connectionID, fromSize, 2);
    read(connectionID, from, stoi(fromSize));

    return string(from, stoi(fromSize)) + ": " + string(buffer, stoi(msgSize));

  } else if (type == 'L') {
    char numUsers[2] = {0};
    char usernameSize[2] = {0};
    char username[BUFFER_SIZE] = {0};

    read(connectionID, numUsers, 2);

    string res = "USERS: "; 
    for (int i = 0; i < stoi(numUsers); i++) {
      read(connectionID, usernameSize, 2);
      read(connectionID, username, stoi(usernameSize));

      res += string(username, stoi(usernameSize)) + " ";
    }
    return res;
  
  } else if (type == 'A') {
    char code[2] = {0};
    read(connectionID, code, 2);
    string codeStr(code, 2);
    if (codeStr == "10") {
      return "Login is OK";
    } else if (codeStr == "20") {
      return "Message sent!";
    } else if (codeStr == "30") {
      return "Logout OK";
    } else {
      return "Other ack code";
    }

  } else if (type == 'E') {
    char code[2] = {0};
    read(connectionID, code, 2);
    string codeStr(code, 2);
    if (codeStr == "10") {
      return "Login error";
    } else if (codeStr == "20") {
      return "Message sent error";
    } else if (codeStr == "30") {
      return "Logout error";
    } else {
      return "Other error code";
    }

  }

  return "This command is not supported: " + string(1, type);
}

void readConcurrent(const int connectionID, const string &otherName,
                    NcursesUI &ui, atomic<bool> &hasExited) {
  while (true) {
    string message = comm::readWithProtocol(connectionID);
     cout << "[" << otherName << "]: " << message << '\n';
    //ui.writeOutput("[" + otherName + "]: " + message);

    if (message == comm::QUIT_COMMAND) {
      hasExited = true;
      break;
    }
  }
}

void writeWithProtocol(const string &message, const int connectionID) {
  char operation = message[0];
  char cont1[BUFFER_SIZE] = {0};
  char cont2[BUFFER_SIZE] = {0};

  auto firstSpace = message.find(' ');
  auto secondSpace = message.find(' ', firstSpace + 1);

  if (operation == 'I') {
    string username(message, firstSpace + 1);
    char usernameSize[3] = {0};
    snprintf(usernameSize,3, "%02d", username.size());

    write(connectionID, &operation, 1);
    write(connectionID, usernameSize, 2);
    write(connectionID, username.c_str(), username.size());
  } else if (operation == 'O') {
    write(connectionID, &operation, 1);
  } else if (operation == 'M') {

    string username(message, firstSpace + 1, secondSpace - firstSpace - 1);
    string propMessage(message, secondSpace + 1);
    char msgSize[3] = {0};
    char usernameSize[3] = {0};

    snprintf(usernameSize, 3, "%02d", username.size());
    snprintf(msgSize, 3, "%02d", propMessage.size());

    write(connectionID, &operation, 1);
    write(connectionID, msgSize, 2);
    write(connectionID, propMessage.c_str(), propMessage.size());
    write(connectionID, usernameSize, 2);
    write(connectionID, username.c_str(), username.size());
  } else if (operation == 'L') {
    write(connectionID, &operation, 1);
  } else if (operation == 'B') {
    string propMessage(message, firstSpace + 1);
    char msgSize[3] = {0};

    snprintf(msgSize, 3, "%02d", propMessage.size());

    write(connectionID, &operation, 1);
    write(connectionID, msgSize, 2);
    write(connectionID, propMessage.c_str(), propMessage.size());
  }
  else {
    cerr << "Operation not supported\n";
  }
}

} // namespace comm