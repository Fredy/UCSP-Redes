#pragma once

#include "ui.hpp"
#include <atomic>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
using namespace std;

namespace comm {
const int PORT = 8889;
const int BUFFER_SIZE = 256;
const int PROTOCOL_BUFFER_SIZE = 5;
const string QUIT_COMMAND = "quit";
const int ROWS = 20;
const int COLS = 20;

int posX = 0;
int posY = 0;

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
  auto thirdSpace = message.find(' ', secondSpace + 1);

  if (operation == 'I') {
    string username(message, firstSpace + 1);
    char usernameSize[3] = {0};
    snprintf(usernameSize,3, "%02d", username.size());

    write(connectionID, &operation, 1);
    //write(connectionID, usernameSize, 2);
    write(connectionID, username.c_str(), 1);
  } else if (operation == 'O') {
    write(connectionID, &operation, 1);
  
  } else if (operation == 'M') {
    string positionCol(message, firstSpace + 1, secondSpace - firstSpace - 1);
    string positionRow(message, secondSpace + 1);
    char colsSize[3] = {0};
    char rowsSize[3] = {0};

    snprintf(colsSize, 3, "%02d", positionCol.size());
    snprintf(rowsSize, 3, "%02d", positionRow.size());

    write(connectionID, &operation, 1);
    write(connectionID, rowsSize, 2);
    write(connectionID, positionRow.c_str(), positionRow.size());
    write(connectionID, colsSize, 2);
    write(connectionID, positionCol.c_str(), positionCol.size());
  } else if (operation == 'L') {
    write(connectionID, &operation, 1);
  } else if (operation == 'B') { 
    string rows(message, firstSpace + 1, secondSpace - firstSpace - 1);
    string str(message, secondSpace + 1, thirdSpace - secondSpace -1 );
    string propMessage(message, thirdSpace + 1);  // matrix content
    char msgSize[3] = {0};
    char rowsSize[3] = {0};
    char strSize[3] = {0};

    snprintf(rowsSize, 3, "%02d", rows.size());
    snprintf(strSize, 3, "%02d", str.size());
    snprintf(msgSize, 3, "%02d", propMessage.size());

    write(connectionID, &operation, 1);
    write(connectionID, rowsSize, 2);
    write(connectionID, rows.c_str(), rows.size());
    write(connectionID, strSize, 2);
    write(connectionID, str.c_str(), str.size());
    write(connectionID, msgSize, 2);
    write(connectionID, propMessage.c_str(), propMessage.size());
  } else if(operation =='U'){
    write(connectionID, &operation, 1);
  } else if( operation == 'W'){
    //string positionCol(message, firstSpace + 1, secondSpace - firstSpace - 1);
    //string positionRow(message, secondSpace + 1);
    char positionCol[3] = {0};
    char positionRow[3] = {0};

    posY -= 1;
    //positionCol = atoi(posY)
    //positionRow = atoi(posX);
    snprintf(positionCol, 3, "%02d", posX);
    snprintf(positionRow, 3, "%02d", posY);
    operation = 'M';
    write(connectionID, &operation, 1);
    write(connectionID, positionCol, 2);
    write(connectionID, positionRow, 2);
    //write(connectionID, rowsSize, 2);
    //write(connectionID, colsSize, 2);
  } else if( operation == 'S'){
    char positionCol[3] = {0};
    char positionRow[3] = {0};

    posY += 1;
    //positionCol = atoi(posY)
    //positionRow = atoi(posX);
    operation = 'M';
    snprintf(positionCol, 3, "%02d", posX);
    snprintf(positionRow, 3, "%02d", posY);

    write(connectionID, &operation, 1);
    write(connectionID, positionCol, 2);
    write(connectionID, positionRow, 2);
  }else if( operation == 'A'){
    char positionCol[3] = {0};
    char positionRow[3] = {0};

    posX -= 1;
    operation = 'M';
    snprintf(positionCol, 3, "%02d", posX);
    snprintf(positionRow, 3, "%02d", posY);

    write(connectionID, &operation, 1);
    write(connectionID, positionCol, 2);
    write(connectionID, positionRow, 2);
  }else if( operation == 'D'){
    char positionCol[3] = {0};
    char positionRow[3] = {0};

    posX += 1;
    operation = 'M';
    snprintf(positionCol, 3, "%02d", posX);
    snprintf(positionRow, 3, "%02d", posY);

    write(connectionID, &operation, 1);
    write(connectionID, positionCol, 2);
    write(connectionID, positionRow, 2);
  } else {
    cerr << "Operation not supported\n";
  }
}

string matrixToStr(vector<vector<string>> matrix){
  string matrixStr = "";
  for (int i = 0; i < ROWS; ++i){
    for (int j = 0; j < COLS; ++j){
      matrixStr += matrix[i][j];
    }   
  }
  return matrixStr;
}

vector<vector<string>> strToMatrix(string strMatrix ){
  vector<vector<string>> matrix(ROWS, vector<string>(COLS, " "));
  for (int i = 0; i < ROWS; ++i){
    for (int j = 0; j < COLS; ++j){
      matrix[i][j] = strMatrix[i*COLS +j];
    }   
  }
  return matrix;
}

} // namespace comm