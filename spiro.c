#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define SIZ 600

static float QUALITY = 0.01;
static float N_ROT = 100;

/* (x, y) is the center
   or     is the outer radius
   ir     is the inner radius
   ir2    is the radius of point hole for the pen */
void
spiro(unsigned x, unsigned y, unsigned or, unsigned ir, unsigned ir2, Color c, uint8_t *P)
{
  double scale = (double)(or - ir) / (double) ir;

  double i = 0.;
  double xp, yp, xi, yi, xP, yP;
  while (i < 2*M_PI * N_ROT) {
    xp = or*cos((double)i) + x;
    yp = or*sin((double)i) + y;
    /* DrawPixel(xp, yp, c); */

    /* center of inner circle */
    xi = ir*cos((double)(M_PI+i)) + xp;
    yi = ir*sin((double)(M_PI+i)) + yp;

    /* DrawPixel(xi, yi, PINK); */

    /* point on inner circle */

    xP = ir2*cos((double)2.0*M_PI - i*scale) + xi;
    yP = ir2*sin((double)2.0*M_PI - i*scale) + yi;

    DrawPixel(xP, yP, c);
    if (P && xP >= 0 && xP <= SIZ && yP >= 0 && yP <= SIZ)
      P[(unsigned)yP*SIZ + (unsigned)xP] = 1;

    i += QUALITY;
  }
}

void
export(uint8_t *P, char *fname, Color c) {
  FILE *fp = fopen(fname, "wb");
  fprintf(fp, "P6 %d %d 255\n", SIZ, SIZ);
  unsigned i = 0;

  uint8_t fg[3] = {c.r, c.g, c.b};
  uint8_t bg[3] = {0,   0,   0};

  for (; i < SIZ*SIZ; ++i) {
    if (P[i])
      fwrite(fg, 3, 1, fp);
    else
      fwrite(bg, 3, 1, fp);
  }

  fclose(fp);
  printf("Exported to %s\n", fname);
}

int
main(void)
{
  InitWindow(SIZ, SIZ, "spiro");

  SetTargetFPS(60);

  uint8_t *plane = malloc(SIZ * SIZ);
  memset(plane, 0, SIZ*SIZ);

  float ir = 200;
  float ir2 = 100;

  while (!WindowShouldClose()) {
    BeginDrawing();

    ClearBackground(BLACK);

    /* if (IsKeyDown(KEY_A)) ir++; */
    /* if (IsKeyDown(KEY_Z)) ir--; */
    /* if (IsKeyDown(KEY_S)) ir2++; */
    /* if (IsKeyDown(KEY_X)) ir2--; */

    /* if (IsKeyPressed(KEY_Q)) QUALITY += 0.001; */
    /* if (IsKeyPressed(KEY_W)) QUALITY -= 0.001; */
    /* if (QUALITY <= 0.0) */
    /*   QUALITY = 0.001; */

    GuiSlider((Rectangle) {0.0, 0.0,  300.0, 30.0}, "",  TextFormat("r: %lf", ir), &ir, 0, 600);
    GuiSlider((Rectangle) {0.0, 35.0, 300.0, 30.0}, "",  TextFormat("r': %lf", ir2), &ir2, 0, ir);
    GuiSlider((Rectangle) {0.0, 70.0, 300.0, 30.0}, "",  TextFormat("q: %lf", QUALITY), &QUALITY, 0.0005, 0.4);
    GuiSlider((Rectangle) {0.0, 105.0, 300.0, 30.0}, "", TextFormat("N: %lf", N_ROT), &N_ROT, 1, 1000);

    if (ir2 > ir)
      ir2 = ir;

    /* DrawText(TextFormat("ir: %d", ir), 0, 0, 16, WHITE); */
    /* DrawText(TextFormat("ir2: %d", ir2), 0, 20, 16, WHITE); */
    /* DrawText(TextFormat("Q: %lf", QUALITY), 0, 40, 16, WHITE); */

    DrawFPS(500, 0);

    if (IsKeyPressed(KEY_E)) {
      spiro(300, 300, 300, ir, ir2, ORANGE, plane);
      export(plane, "export.ppm", ORANGE);
    } else {
      spiro(300, 300, 300, ir, ir2, ORANGE, NULL);
    }

    EndDrawing();
  }
}
