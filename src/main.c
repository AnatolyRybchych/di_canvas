#include <stdio.h>
#include <di_canvas.h>

int main(void){
    DiCanvas canvas = di_alloc_canvas(800, 800);

    di_clear(&canvas, (DiColor){.r = 0, .g = 0, .b = 0, .a = 255});

    di_draw_rect(&canvas, DI_POINT(790, -10), DI_SIZE(100, 100), DI_COLOR_RED);

    int err = di_dump_bmp(&canvas, "dump.bmp");
    di_free_canvas(&canvas);

    return err;
}
