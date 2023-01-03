#ifndef DI_CANVAS_H_
#define DI_CANVAS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

#define DI_MIN(a,b) (((a)<(b))?(a):(b))
#define DI_MAX(a,b) (((a)>(b))?(a):(b))

#define DI_COLOR(red, green, blue, alpha) (DiColor){.r = red, .g = green, .b = blue, .a = alpha}
#define DI_POINT(x, y) (DiPoint){x, y}
#define DI_SIZE(width, height) (DiSize){width, height}

#define DI_COLOR_RED DI_COLOR(255, 0, 0, 0)
#define DI_COLOR_GREEN DI_COLOR(0, 255, 0, 0)
#define DI_COLOR_BLUE DI_COLOR(0, 0, 255, 0)
#define DI_COLOR_TRANSPARENT DI_COLOR(0, 0, 0, 255)

#define di_alloc_canvas(width, height) di_create_canvas(width, height, malloc(width * height * sizeof(DiColor)), free)

typedef struct DiCanvas DiCanvas;
typedef struct DiColor DiColor;
typedef struct DiPoint DiPoint;
typedef struct DiSize DiSize;
typedef void (*DiBlendFunc)(DiColor *dst, const DiColor *src);
typedef void (*DiFreeFunc)(void *pixels);

//enum DiBlend
typedef uint32_t DiBlend;

DiCanvas di_create_canvas(uint32_t width, uint32_t height, DiColor *pixels, DiFreeFunc free);

//if canvas is NULL instantly returns
void di_free_canvas(DiCanvas *canvas);

//returns errno if feiled
int di_dump_bmp(const DiCanvas *canvas, const char *filename);

void di_set_blend_mode(DiCanvas *canvas, DiBlend blend);
DiBlendFunc di_blend_func(DiBlend blend);

void di_clear(DiCanvas *canvas, DiColor color);
void di_draw_rect(DiCanvas *canvas, DiPoint p1, DiSize size, DiColor color);

struct DiCanvas{
    uint32_t width;
    uint32_t height;

    DiColor *pixels;
    DiFreeFunc free;
    DiBlendFunc blend_func;
};

struct DiColor{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

struct DiPoint{
    int x, y;
};

struct DiSize{
    uint32_t w, h;
};

enum DiBlend{
    DI_BLEND_SET_SRC,
    DI_BLEND_SET_SRC_COLOR,
    DI_BLEND_SET_SRC_ALPHA,

    DI_COUNT_BLEND_MODES,
};

#endif //DI_CANVAS_H_