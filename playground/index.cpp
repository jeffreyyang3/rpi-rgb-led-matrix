#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;
string inputLine;

int main() {
  int num, fd;
  char buffer[BUFSIZ];
  while (1) {
    cout << "loop" << endl;
    fd = open("../myPipe", O_RDONLY);
    int size = read(fd, buffer, BUFSIZ);
    cout << buffer;
    close(fd);
  }
  return 0;
}
