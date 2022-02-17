// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how to use the library.
// For more examples, look at demo-main.cc
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"

#include <bitset>
#include <iostream>
#include <math.h>
#include <regex>
#include <signal.h>
#include <stdio.h>
#include <sw/redis++/redis++.h>
#include <unistd.h>
#define msToSec 1000000
using namespace std;
using namespace sw::redis;
using rgb_matrix::Canvas;
using rgb_matrix::RGBMatrix;
pthread_mutex_t writeLock;
bool blue = true;
volatile bool interrupt_received = false;
static void InterruptHandler(int signo) { interrupt_received = true; }

struct matrixData {
  string data;
  int rows;
  int cols;
};
// g++ -std=c++17 index.cpp -lhiredis -lredis++ -lpthread
// -I/home/pi/redis-plus-plus/build && ./a.out
struct matrixData matrixDataVals(string str) {
  regex e("((?:0|1)*),(\\d+),(\\d+)");
  smatch sm;
  regex_match(str, sm, e);
  matrixData result;
  result.data = sm[1];
  result.rows = stoi(sm[2]);
  result.cols = stoi(sm[3]);
  return result;
};

void write7x5(int iStart, int jStart, Canvas *canvas) {
  string infoString =
      "000000000000000000000000000000000000000000011110000111000111100001110000"
      "000000000000010001001000100100010010001000000000000000010001001000100100"
      "010010001000000000000000011111001111100111110011111000000000000000010001"
      "001000100100010010001000000000000000010001001000100100010010001000000000"
      "000000011110001000100111100010001000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000011110001111"
      "100111110011111001000100000000010001001000100100010010000001000100000000"
      "010001001000100100010010000000101000000000011111001000100100010011111000"
      "010000000000010001001000100100010010000000010000000000010001001000100100"
      "010010000000010000000000011110001111100111110011111000010000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000100000111000111110010001001111100111110001100001000100000010010"
      "001001000000100000010100000000100000010010001001000000100000000100000001"
      "000111110011111001111000111110000100000010000000010000001000000100100010"
      "000100000100000000010000001000000100100010011111001111100111110000001001"
      "111100111110000000000000000000000000000000000000000000,27,42";
  auto parsed = matrixDataVals(infoString);
  string bitString = parsed.data;
  int rows = parsed.rows;
  int cols = parsed.cols;
  // 9 rows 28 cols
  pthread_mutex_lock(&writeLock);
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      bool val = bitString[(i * cols) + j] == '1';
      if (blue)
        canvas->SetPixel(j + jStart, i + iStart, 0, val ? 255 : 0,
                         val ? 255 : 0);
      else {
        canvas->SetPixel(j + jStart, i + iStart, val ? 255 : 0, 0, 0);
      }
    }
  }
  pthread_mutex_unlock(&writeLock);
}

static void DrawOnCanvas(Canvas *canvas) {
  /*
   * Let's create a simple animation. We use the canvas to draw
   * pixels. We wait between each step to have a slower animation.
   */
  canvas->Fill(0, 0, 0);

  while (true) {
    if (interrupt_received) {
      cout << "interrupt received" << endl;
      return;
    };
    write7x5(0, 0, canvas);
    usleep(msToSec / 3);
  }
}
void processRedisMessage(string channel, string msg) {
  cout << endl << "Channel is " + channel << endl << "Msg is " + msg << endl;

  pthread_mutex_lock(&writeLock);
  if (msg == "c")
    blue = !blue;
  pthread_mutex_unlock(&writeLock);
}
void *redisSubscriberThreadFunc(void *threadid) {
  cout << "thread id " + (long)threadid << endl;
  string input;
  auto redis = Redis("tcp://127.0.0.1:6379");
  auto sub = redis.subscriber();
  sub.subscribe({"dataChannel"});
  sub.on_message(processRedisMessage);

  while (!interrupt_received) {
    try {
      sub.consume();
    } catch (const Error &err) {
      cout << err.what() << endl;
    }
  }
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  RGBMatrix::Options defaults;
  defaults.hardware_mapping = "regular"; // or e.g. "adafruit-hat"
  defaults.rows = 32;
  defaults.chain_length = 1;
  defaults.parallel = 1;
  defaults.show_refresh_rate = true;

  Canvas *canvas = RGBMatrix::CreateFromFlags(&argc, &argv, &defaults);
  if (canvas == NULL)
    return 1;

  // It is always good to set up a signal handler to cleanly exit when we
  // receive a CTRL-C for instance. The DrawOnCanvas() routine is looking
  // for that.
  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  pthread_t redisSubscriberThread;
  pthread_create(&redisSubscriberThread, NULL, &redisSubscriberThreadFunc,
                 NULL);
  DrawOnCanvas(canvas); // Using the canvas.
  // Animation finished. Shut down the RGB matrix.
  canvas->Clear();
  delete canvas;

  return 0;
}
