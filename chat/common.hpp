#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>
using namespace std;

namespace comm {
const int PORT = 2999;
const int BUFFER_SIZE = 256;
const int PROTOCOL_BUFFER_SIZE = 5;
const string QUIT_COMMAND = "quit";

string readWithProtocol(const int connectionID) {
  char buffer[BUFFER_SIZE] = {0};
  char bufferProtocol[PROTOCOL_BUFFER_SIZE] = {0};

  int n = read(connectionID, bufferProtocol, 5);

  // Check for errors.
  if (n < 0) {
    perror("ERROR:PROTOCOL reading from socket");
  }

  int numChars;
  try {
    numChars = stoi(bufferProtocol);
  } catch (const exception &e) {
    cerr << ">> Something gone wrong: " << e.what() << '\n';
    cerr << ">> BuffProtocol: '" << bufferProtocol << "'\n";
    return "";
  }

  n = read(connectionID, buffer, numChars);

  // Check for errors.
  if (n < 0) {
    perror("ERROR:MESSAGE reading from socket");
  }

  return {buffer};
}

void readThread(const int connectionID, const string &otherName) {
  while (true) {
    const string inMessage = comm::readWithProtocol(connectionID);
    cout << "[" << otherName << "]: " << inMessage << '\n';

    if (inMessage == comm::QUIT_COMMAND) {
      break;
    }
  }
}

void writeWithProtocol(const string &message, const int connectionID) {
  char bufferProtocol[PROTOCOL_BUFFER_SIZE] = {0};
  sprintf(bufferProtocol, "%04d", message.size());

  int n = write(connectionID, bufferProtocol, 5);

  // Check for errors.
  if (n < 0) {
    perror("ERROR:PROTOCOL writing to socket");
  }

  n = write(connectionID, message.c_str(), message.size());

  // Check for errors.
  if (n < 0) {
    perror("ERROR:MESSAGE writing to socket");
  }
}

} // namespace comm