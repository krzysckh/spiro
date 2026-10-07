#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define SIZ 600
#define ANIM_STEP_SIZE 100
#define SCALE(or, ir) ((double)(or - ir) / (double) ir)

static float QUALITY = 0.01;
static float N_ROT = 100;

static inline void
spiro_do_step(double i, unsigned x, unsigned y, unsigned or, unsigned ir, unsigned ir2, double scale, double *xP, double *yP)
{
  double xp, yp, xi, yi;

  xp = or*cos((double)i) + x;
  yp = or*sin((double)i) + y;

  /* center of inner circle */
  xi = ir*cos((double)(M_PI+i)) + xp;
  yi = ir*sin((double)(M_PI+i)) + yp;

  /* point on inner circle */
  *xP = ir2*cos((double)2.0*M_PI - i*scale) + xi;
  *yP = ir2*sin((double)2.0*M_PI - i*scale) + yi;
}

/* (x, y) is the center
   or     is the outer radius
   ir     is the inner radius
   ir2    is the radius of point hole for the pen */
void
spiro(unsigned x, unsigned y, unsigned or, unsigned ir, unsigned ir2, Color c, uint8_t *P)
{
  double scale = SCALE(or, ir);

  double i = 0.;
  double xP, yP;
  while (i < 2*M_PI * N_ROT) {
    spiro_do_step(i, x, y, or, ir, ir2, scale, &xP, &yP);

    DrawPixel(xP, yP, c);
    if (P && xP >= 0 && xP <= SIZ && yP >= 0 && yP <= SIZ)
      P[(unsigned)yP*SIZ + (unsigned)xP] = 1;

    i += QUALITY;
  }
}

void
spiro_animate(unsigned x, unsigned y, unsigned or, unsigned ir, unsigned ir2, Color c)
{
  double scale = SCALE(or, ir);
  unsigned ctr = 0;

  BeginDrawing();
  ClearBackground(BLACK);
  EndDrawing();

  double i = 0.;
  double xP, yP;

  BeginDrawing();
  while (i < 2*M_PI * N_ROT && !WindowShouldClose()) {
    ctr++;
    if (ctr >= ANIM_STEP_SIZE) {
      ctr = 0;
      EndDrawing();
      BeginDrawing();
    }

    spiro_do_step(i, x, y, or, ir, ir2, scale, &xP, &yP);
    DrawPixel(xP, yP, c);

    i += QUALITY;
  }
  EndDrawing();
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
    if (IsKeyDown(KEY_A))
      spiro_animate(300, 300, 300, ir, ir2, ORANGE);

    BeginDrawing();

    ClearBackground(BLACK);

    GuiSlider((Rectangle) {0.0, 0.0,  300.0, 30.0}, "",  TextFormat("r: %lf", ir), &ir, 0, 600);
    GuiSlider((Rectangle) {0.0, 35.0, 300.0, 30.0}, "",  TextFormat("r': %lf", ir2), &ir2, 0, ir);
    GuiSlider((Rectangle) {0.0, 70.0, 300.0, 30.0}, "",  TextFormat("q: %lf", QUALITY), &QUALITY, 0.0005, 0.4);
    GuiSlider((Rectangle) {0.0, 105.0, 300.0, 30.0}, "", TextFormat("N: %lf", N_ROT), &N_ROT, 1, 1000);

    if (ir2 > ir)
      ir2 = ir;

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
