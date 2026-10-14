#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define nil NULL
#define SIZ 600
#define SCALE(or, ir) ((double)(or - ir) / (double) ir)

typedef struct {
  double x, y;
} vec2;

static float QUALITY = 0.01;
static float N_ROT = 100;

static inline void
spiro_do_step(double i, unsigned x, unsigned y, unsigned or, unsigned ir, unsigned ir2, double scale, vec2 *P, vec2 *ipt)
{
  double xp, yp, xi, yi;

  /* point on outer circle */
  xp = or*cos((double)i) + x;
  yp = or*sin((double)i) + y;

  /* center of inner circle */
  xi = ir*cos((double)(M_PI+i)) + xp;
  yi = ir*sin((double)(M_PI+i)) + yp;

  if (ipt)
    ipt->x = xi, ipt->y = yi;

  /* point on inner circle */
  P->x = ir2*cos((double)2.0*M_PI - i*scale) + xi;
  P->y = ir2*sin((double)2.0*M_PI - i*scale) + yi;
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
  vec2 pt;
  while (i < 2*M_PI * N_ROT) {
    spiro_do_step(i, x, y, or, ir, ir2, scale, &pt, nil);

    DrawPixel(pt.x, pt.y, c);
    if (P && pt.x >= 0 && pt.x <= SIZ && pt.y >= 0 && pt.y <= SIZ)
      P[(unsigned)pt.y*SIZ + (unsigned)pt.x] = 1;

    i += QUALITY;
  }
}

static void
draw_cogs(vec2 ipt, unsigned ir, unsigned ir2)
{
  DrawCircleLines(ipt.x, ipt.y, ir, GRAY);
  DrawCircleLines(ipt.x, ipt.y, ir2, LIGHTGRAY);
  DrawCircleLines(SIZ/2.f, SIZ/2.f, SIZ/2.f, RED);
}

void
spiro_animate(unsigned x, unsigned y, unsigned or, unsigned ir, unsigned ir2, Color c)
{
  RenderTexture2D t = LoadRenderTexture(SIZ, SIZ);

  double scale = SCALE(or, ir);
  unsigned ctr = 0;
  float step_size = 10;

  double i = 0.;
  vec2 pt, ipt;

  Rectangle sl = {0.0, 0.0, 300.0, 30.0};

  BeginTextureMode(t);
  ClearBackground(BLACK);
  EndTextureMode();

  while (i < 2*M_PI * N_ROT && !WindowShouldClose()) {
    ctr++;
    if (ctr >= step_size) {
      ctr = 0;
      BeginDrawing();
      ClearBackground(BLACK);
      DrawTexturePro(t.texture,
                     (Rectangle){0, 0, t.texture.width, -t.texture.height}, /* WHY is the texture backwards */
                     (Rectangle){SIZ/2.f, SIZ/2.f, t.texture.width, t.texture.height},
                     (Vector2){t.texture.width/2.f, t.texture.height/2.f}, 0, WHITE);
      DrawCircle(ipt.x, ipt.y, 2, MAGENTA);
      DrawCircle(pt.x, pt.y, 5, RED); /* even though the point is right */
      draw_cogs(ipt, ir, ir2);
      if (CheckCollisionPointRec(GetMousePosition(), sl))
        GuiSlider(sl, "", TextFormat("step_size: %lf", step_size), &step_size, 1, 100);
      EndDrawing();
    }

    spiro_do_step(i, x, y, or, ir, ir2, scale, &pt, &ipt);
    BeginTextureMode(t);
    DrawPixel(pt.x, pt.y, c);
    EndTextureMode();

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

  float ir = 84.f;
  float ir2 = 64.f;

  while (!WindowShouldClose()) {
    if (IsKeyDown(KEY_A))
      spiro_animate(300, 300, 300, ir, ir2, ORANGE);

    BeginDrawing();

    ClearBackground(BLACK);

    GuiSlider((Rectangle) {0.0, 0.0,  300.0, 30.0}, "",  TextFormat("r: %lf", ir), &ir, 0, SIZ/2.f);
    GuiSlider((Rectangle) {0.0, 35.0, 300.0, 30.0}, "",  TextFormat("r': %lf", ir2), &ir2, 0, ir);
    GuiSlider((Rectangle) {0.0, 70.0, 300.0, 30.0}, "",  TextFormat("q: %lf", QUALITY), &QUALITY, 0.0005, 0.1);
    GuiSlider((Rectangle) {0.0, 105.0, 300.0, 30.0}, "", TextFormat("N: %lf", N_ROT), &N_ROT, 1, 1000);

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      vec2 ipt, _;
      spiro_do_step(0, SIZ/2.f, SIZ/2.f, SIZ/2.f, ir, ir2, SCALE(SIZ/2.f, ir), &_, &ipt);
      draw_cogs(ipt, ir, ir2);
    }

    if (ir2 > ir)
      ir2 = ir;

    DrawFPS(500, 0);

    if (IsKeyPressed(KEY_E)) {
      spiro(300, 300, 300, ir, ir2, ORANGE, plane);
      export(plane, "export.ppm", ORANGE);
    } else {
      spiro(300, 300, 300, ir, ir2, ORANGE, nil);
    }

    EndDrawing();
  }
}
