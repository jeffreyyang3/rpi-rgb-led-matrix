#include <iostream>
#include <sw/redis++/redis++.h>
using namespace sw::redis;
using namespace std;

int main() {
  ConnectionOptions opts1;
  auto redis = Redis("tcp://127.0.0.1:6379");
  auto sub = redis.subscriber();

  sub.subscribe({"cool", "man"});
  sub.on_message([](string channel, string msg) {
    cout << "received message: " << msg << "from channel:" << channel << endl;
  });

  cout << "runs" << endl;
  while (1) {
    cout << "loop" << endl;
    try {
      sub.consume();
    } catch (const Error &err) {
      cout << "error !!!" << endl;
      cout << err.what() << endl;
    }
  }
  return 0;
}