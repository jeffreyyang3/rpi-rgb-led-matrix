#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
using namespace std;
string inputLine;
string bufferToStdString(char *buffer, int bytes) {
  vector<char> chars;
  for (int i = 0; i < bytes; i++) {
    chars.push_back(buffer[i]);
  }
  return string(chars.begin(), chars.end());
}
int count = 0;
int main() {
  int num, fd;
  char buffer[BUFSIZ];
  char pipePath[100];
  strcpy(pipePath, getenv("HOME"));
  strcat(pipePath, "/myPipe");
  cout << "started" << endl;
  while (1) {
    cout << "count is " << count++ << endl;
    fd = open(pipePath, O_RDWR);
    int bytesRead = read(fd, buffer, BUFSIZ);
    if (bytesRead) {
      cout << bufferToStdString(buffer, bytesRead) << endl;
    } else {
      cout << "no bytes read" << endl;
    }
    cout << endl;
    close(fd);
  }
  return 0;
}
