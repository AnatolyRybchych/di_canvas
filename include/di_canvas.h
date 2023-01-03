#ifndef DI_CANVAS_H_
#define DI_CANVAS_H_

#include <stdint.h>

typedef struct DiCanvas DiCanvas;
typedef struct DiColor DiColor;

DiCanvas di_create_canvas(uint32_t width, uint32_t height, DiColor *pixels, void (*free)(void* pixels));
void di_free_canvas(DiCanvas *canvas);

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