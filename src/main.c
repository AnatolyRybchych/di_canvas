#include <stdio.h>
#include <string.h>
#include <di_canvas.h>

int main(void){
    DiCanvas canvas = di_alloc_canvas(800, 800);

    di_clear(&canvas, (DiColor){.r = 0, .g = 0, .b = 0, .a = 255});

    //di_draw_rect(&canvas, DI_POINT(10, 10), DI_SIZE(100, 100), DI_COLOR_RED);

    //di_draw_line(&canvas, DI_POINT(-10, -10), DI_POINT(200, 500), DI_COLOR_GREEN);

    di_draw_triangle(&canvas, DI_POINT(100, 100), DI_POINT(400, 300), DI_POINT(250, 500), DI_COLOR_GREEN);

    int err = di_dump_bmp(&canvas, "dump.bmp");
    if(err){
        fprintf(stderr, "ERROR: %s\n", strerror(err));
    }
    di_free_canvas(&canvas);

    return err;
}
