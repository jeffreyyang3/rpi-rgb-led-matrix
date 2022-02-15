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
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#define msToSec 1000000
using namespace std;
using rgb_matrix::Canvas;
using rgb_matrix::RGBMatrix;
pthread_mutex_t lock;
bool blue = true;
volatile bool interrupt_received = false;
static void InterruptHandler(int signo) { interrupt_received = true; }

void write7x5(int iStart, int jStart, Canvas *canvas) {
  string bitString =
      "000000000000000000000000000000000000111100011111001111100111110010001001"
      "000100100010010001001000000100010010001001000100100010010000000101000111"
      "110010001001000100111110000100001000100100010010001001000000001000010001"
      "001000100100010010000000010000111100011111001111100111110000100000000000"
      "000000000000000000000000000";
  // 9 rows 28 cols
  pthread_mutex_lock(&lock);
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 35; j++) {
      bool val = bitString[(i * 35) + j] == '1';
      if (blue)
        canvas->SetPixel(j + jStart, i + iStart, 0, val ? 255 : 0,
                         val ? 255 : 0);
      else {
        canvas->SetPixel(j + jStart, i + iStart, val ? 255 : 0, 0, 0);
      }
    }
  }
  pthread_mutex_unlock(&lock);
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
    // usleep(2 * msToSec);
  }
}

void *stdinReader(void *threadid) {
  cout << "thread id " + (long)threadid << endl;
  string input;
  while (!interrupt_received) {
    getline(cin, input);
    pthread_mutex_lock(&lock);
    if (input == "c") {
      blue = !blue;
    } else {
      cout << "input was " + input << endl;
    }
    pthread_mutex_unlock(&lock);
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

  pthread_t readerThread;
  pthread_create(&readerThread, NULL, &stdinReader, NULL);
  DrawOnCanvas(canvas); // Using the canvas.
  // Animation finished. Shut down the RGB matrix.
  canvas->Clear();
  delete canvas;

  return 0;
}
