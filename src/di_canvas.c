#include <di_canvas.h>

#define BI_RGB 0
#define BM_HDR_SIZE 14
#define BI_HDR_SIZE 40
#define BMP_HDR_SIZE (BM_HDR_SIZE + BI_HDR_SIZE)

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

int di_dump_bmp(const DiCanvas *canvas, const char *filename){
    errno = 0;
    FILE *file = fopen(filename, "w");
    if(errno != 0){
        return errno;
    }

    char     bm_magic[2] = {'B', 'M'};
    uint32_t bm_size = canvas->width * canvas->height * sizeof(DiColor) + BMP_HDR_SIZE;
    uint32_t bm_reserved = 0;
    uint32_t bm_data_addr = BMP_HDR_SIZE;

    uint32_t bi_size = BI_HDR_SIZE;
    uint32_t bi_widht = canvas->width;
    uint32_t bi_height = canvas->height;
    uint16_t bi_color_plains = 1;
    uint32_t bi_bits = 32;
    uint32_t bi_compression = BI_RGB;
    uint32_t bi_data_size = canvas->width * canvas->height * sizeof(DiColor);
    int32_t  bi_ppm_horisontal = 2000;
    int32_t  bi_ppm_vetical = 2000;
    uint32_t bi_colors = 0;
    uint32_t bi_colors_important = 0;

    #define __WRT(target) fwrite(&target, sizeof(target), 1, file)
    fwrite(bm_magic, 1, 2, file);
    __WRT(bm_size);
    __WRT(bm_reserved);
    __WRT(bm_data_addr);

    __WRT(bi_size);
    __WRT(bi_widht);
    __WRT(bi_height);
    __WRT(bi_color_plains);
    __WRT(bi_bits);
    __WRT(bi_compression);
    __WRT(bi_data_size);
    __WRT(bi_ppm_horisontal);
    __WRT(bi_ppm_vetical);
    __WRT(bi_colors);
    __WRT(bi_colors_important);

    fwrite(canvas->pixels, sizeof(DiColor), canvas->width * canvas->height, file);

    return errno;
}