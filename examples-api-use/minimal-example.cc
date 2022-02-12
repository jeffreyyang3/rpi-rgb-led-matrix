// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how to use the library.
// For more examples, look at demo-main.cc
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>
#include <bitset>
#include <iostream>
#define msToSec 1000000

using namespace std;
using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

void write7x5 (int iStart, int jStart, Canvas *canvas) {
  string aBitString = "11110100011000111111100011000111110";
  for (int i = 0; i < 7; i++) {
    for (int j = 0; j < 5; j++) {
      bool val = aBitString[(i * 5) + j] == '1';
      canvas->SetPixel(j + jStart, i + iStart, val ? 255 : 0, 0, 0);
    }
  }
}

static void DrawOnCanvas(Canvas *canvas) {
  /*
   * Let's create a simple animation. We use the canvas to draw
   * pixels. We wait between each step to have a slower animation.
   */
  canvas->Fill(0, 0, 0);
  int center_x = canvas->width() / 2;

  while (true) {
    if (interrupt_received) {
      cout << "interrupt received" << endl;
      return;
    };
    write7x5(2,2, canvas);
    usleep(2 * msToSec);
  }

}



int main(int argc, char *argv[]) {
  RGBMatrix::Options defaults;
  defaults.hardware_mapping = "regular";  // or e.g. "adafruit-hat"
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

  DrawOnCanvas(canvas);    // Using the canvas.

  // Animation finished. Shut down the RGB matrix.
  canvas->Clear();
  delete canvas;

  return 0;
}
