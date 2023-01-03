#include <stdio.h>
#include <di_canvas.h>

int main(void){
    DiCanvas canvas = di_alloc_canvas(800, 800);

    int err = di_dump_bmp(&canvas, "dump.bmp");
    di_free_canvas(&canvas);

    return err;
}
