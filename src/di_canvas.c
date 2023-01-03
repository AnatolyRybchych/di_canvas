#include <di_canvas.h>
#include <stdlib.h>

DiCanvas di_create_canvas(uint32_t width, uint32_t height, DiColor *pixels, void (*free)(void* pixels)){
    DiCanvas result = {
        .width = width,
        .height = height,
        .pixels = pixels,
        .free = free
    };
    return result;
}

void di_free_canvas(DiCanvas *canvas){
    if(canvas == NULL) return;

    if(canvas->free != NULL){
        canvas->free(canvas->pixels);
    }
}
