#ifndef DI_CANVAS_H_
#define DI_CANVAS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define di_alloc_canvas(width, height) di_create_canvas(width, height, malloc(width * height * sizeof(DiColor)), free)

typedef struct DiCanvas DiCanvas;
typedef struct DiColor DiColor;

DiCanvas di_create_canvas(uint32_t width, uint32_t height, DiColor *pixels, void (*free)(void* pixels));

//if canvas is NULL instantly returns
void di_free_canvas(DiCanvas *canvas);

//returns errno if feiled
int di_dump_bmp(const DiCanvas *canvas, const char *filename);

void di_clear(DiCanvas *canvas, DiColor color);

struct DiCanvas{
    uint32_t width;
    uint32_t height;

    DiColor *pixels;
    void (*free)(void *);
};

struct DiColor{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

#endif //DI_CANVAS_H_