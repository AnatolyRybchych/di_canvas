#ifndef DI_CANVAS_H_
#define DI_CANVAS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <memory.h>

#define DI_MIN(a,b) (((a)<(b))?(a):(b))
#define DI_MAX(a,b) (((a)>(b))?(a):(b))
#define DI_SIGN(a) ((a) > 0 ? 1 : -1)
#define DI_SWAP(a, b) do{typeof(a) tmp = (a); (a) = (b); (b) = tmp;}while(0)
#define DI_IN_BOUNDS(canvas, x, y) ((x) < 0 || (x) >= (int64_t)(canvas).width || (y) < 0 || y >= (int64_t)(canvas).height)
#define DI_PIXEL(canvas, x, y) ((canvas).pixels[((canvas).height - (y) - 1) * (canvas).width + (x)])
#define DI_PIXEL_SAFE(canvas, x, y) (*(DI_IN_BOUNDS(canvas, x, y) ? di_trash_can() : &DI_PIXEL(canvas, x, y)))

#define DI_COLOR(red, green, blue, alpha) (DiColor){.r = red, .g = green, .b = blue, .a = alpha}
#define DI_POINT(x, y) (DiPoint){x, y}
#define DI_SIZE(width, height) (DiSize){width, height}

#define DI_COLOR_RED DI_COLOR(255, 0, 0, 0)
#define DI_COLOR_GREEN DI_COLOR(0, 255, 0, 0)
#define DI_COLOR_BLUE DI_COLOR(0, 0, 255, 0)
#define DI_COLOR_TRANSPARENT DI_COLOR(0, 0, 0, 255)

#define di_alloc_canvas(width, height) di_create_canvas(width, height, malloc(width * height * sizeof(DiColor)), free)
#define di_alloc_canvas_copy(src) di_create_canvas_copy(malloc((src)->width * (src)->height * sizeof(DiColor)), free, src)
#define di_alloc_canvas_copy_blured(src, blur_radius) di_create_canvas_copy_blured(malloc((src)->width * (src)->height * sizeof(DiColor)), free, src, blur_radius)

typedef struct DiCanvas DiCanvas;
typedef struct DiColor DiColor;
typedef struct DiPoint DiPoint;
typedef struct DiPointF DiPointF;
typedef struct DiSize DiSize;
typedef struct DiSizeF DiSizeF;
typedef void (*DiBlendFunc)(DiColor *dst, const DiColor *src);
typedef void (*DiFreeFunc)(void *pixels);

//enum DiBlend
typedef uint32_t DiBlend;

DiCanvas di_create_canvas(uint32_t width, uint32_t height, DiColor *pixels, DiFreeFunc free);

DiCanvas di_create_canvas_copy(DiColor *pixels, DiFreeFunc free, const DiCanvas *src);

//blur_radius <= 5
DiCanvas di_create_canvas_copy_blured(DiColor *pixels, DiFreeFunc free, const DiCanvas *src, uint8_t blur_radius);

//if canvas is NULL instantly returns
void di_free_canvas(DiCanvas *canvas);

//returns errno if feiled
int di_dump_bmp(const DiCanvas *canvas, const char *filename);

//canvas cannot be NULL
void di_set_blend_mode(DiCanvas *canvas, DiBlend blend);
DiBlendFunc di_blend_func(DiBlend blend);

//canvas cannot be NULL
void di_clear(DiCanvas *canvas, DiColor color);

//canvas cannot be NULL
void di_draw_rect(DiCanvas *canvas, DiPoint p1, DiSize size, DiColor color);

//canvas cannot be NULL
void di_draw_line(DiCanvas *canvas, DiPoint p1, DiPoint p2, DiColor color);

//canvas cannot be NULL
void di_draw_triangle(DiCanvas *canvas, DiPoint p1, DiPoint p2, DiPoint p3, DiColor color);

//canvas cannot be NULL
void di_stroke_circle(DiCanvas *canvas, DiPoint p, uint32_t r, DiColor color);

//canvas cannot be NULL
void di_fill_circle(DiCanvas *canvas, DiPoint p, uint32_t r, DiColor color);

//target, p1 and p2 cannot be NULL
DiPoint *di_nearest_to(const DiPoint *target, DiPoint *p1, DiPoint *p2);

void di_enum_line_points(DiPoint p1, DiPoint p2, void (*enum_proc)(int x, int y, void *data), void *data);

//y = kx + b for each x
void di_enum_line_points_unique_x(DiPoint p1, DiPoint p2, void (*enum_proc)(int x, int y, void *data), void *data);

//x = (y - b) / k for each y
void di_enum_line_points_unique_y(DiPoint p1, DiPoint p2, void (*enum_proc)(int x, int y, void *data), void *data);

DiColor *di_trash_can(void);

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

struct DiPointF{
    float x, y;
};

struct DiSizeF{
    float x, y;
};

enum DiBlend{
    DI_BLEND_SET_SRC,
    DI_BLEND_SET_SRC_COLOR,
    DI_BLEND_SET_SRC_ALPHA,

    DI_COUNT_BLEND_MODES,
};

#endif //DI_CANVAS_H_