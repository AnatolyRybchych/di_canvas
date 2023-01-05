#include <di_canvas.h>

#define BI_RGB 0
#define BM_HDR_SIZE 14
#define BI_HDR_SIZE 40
#define BMP_HDR_SIZE (BM_HDR_SIZE + BI_HDR_SIZE)

#define CLAMP_TO_ZERO(val) do {if(val < 0) val = 0;} while (0)
#define CLAMP_TO_255(val) do {if(val > 255) val = 255;} while (0)

static void blend_set_src(DiColor *dst, const DiColor *src);
static void blend_set_src_color(DiColor *dst, const DiColor *src);
static void blend_set_src_alpha(DiColor *dst, const DiColor *src);

DiCanvas di_create_canvas(uint32_t width, uint32_t height, DiColor *pixels, void (*free)(void* pixels)){
    DiCanvas result = {
        .width = width,
        .height = height,
        .pixels = pixels,
        .free = free,
        .blend_func = di_blend_func(DI_COUNT_BLEND_MODES),
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
    uint16_t bi_bits = 32;
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

    for (uint64_t i = 0; i < canvas->width * canvas->height; i++){
        fwrite(&canvas->pixels[i].b, 1, 1, file);
        fwrite(&canvas->pixels[i].g, 1, 1, file);
        fwrite(&canvas->pixels[i].r, 1, 1, file);
        fwrite(&canvas->pixels[i].a, 1, 1, file);
    }

    return errno;
}

void di_clear(DiCanvas *canvas, DiColor color){
    for (uint64_t i = 0; i < canvas->width * canvas->height; i++){
        canvas->pixels[i] = color;
    }
}

void di_set_blend_mode(DiCanvas *canvas, DiBlend blend){
    canvas->blend_func = di_blend_func(blend);
}

DiBlendFunc di_blend_func(DiBlend blend){
    switch (blend){
    case DI_BLEND_SET_SRC: return blend_set_src;
    case DI_BLEND_SET_SRC_COLOR: return blend_set_src_color;
    case DI_BLEND_SET_SRC_ALPHA: return blend_set_src_alpha;
    default: return blend_set_src;
    }
}

void di_draw_rect(DiCanvas *canvas, DiPoint point, DiSize size, DiColor color){
    if(point.x < 0){
        if((uint32_t)-point.x > size.w) return;
        size.w += point.x;
        point.x = 0;
    }

    if(point.y < 0){
        if((uint32_t)-point.y > size.h) return;
        size.h += point.y;
        point.y = 0;
    }

    int fw = (int)canvas->width - point.x;
    int fh = (int)canvas->height - point.y;
    DiSize fit = {
        .w = fw > 0 ? fw : 0,
        .h = fh > 0 ? fh : 0
    };

    fit.w = DI_MIN(fit.w, size.w);
    fit.h = DI_MIN(fit.h, size.h);

    for (uint64_t x = 0; x < fit.w; x++){
        for (uint64_t y = 0; y < fit.h; y++){
            DiColor *pixel = &DI_PIXEL(*canvas, x + point.x, y + point.y);
            canvas->blend_func(pixel, &color);
        }
    }
}

void di_draw_line(DiCanvas *canvas, DiPoint p1, DiPoint p2, DiColor color){
    int dx = p2.x - p1.x;
    int sx = DI_SIGN(dx);

    int dy = p2.y - p1.y;
    int sy = DI_SIGN(dy);

    if(dx == 0){//draw vertical line
        for (int y = p1.y; y != p2.y + sy; y += sy){
            canvas->blend_func(&DI_PIXEL_SAFE(*canvas, p1.x, y), &color);
        }
        return false;
    }

    int abs_dx = abs(dx);
    int abs_dy = abs(dy);
    int points_to_draw = DI_MAX(abs_dx, abs_dy);

    int x = p1.x;
    int y = p1.y;

    int b = y - x * dy / dx;

    while (points_to_draw){
        canvas->blend_func(&DI_PIXEL_SAFE(*canvas, x, y), &color);

        int xn = x + sx;
        int yn = xn * dy / dx  + b;

        int offset_yn = abs(y - yn);

        if(offset_yn == 0){
            x += sx;
        }
        else if(offset_yn == 1){
            y += sy;
            x += sx;
        }
        else{
            y += sy;
        }
        points_to_draw --;
    }
}

DiPoint *di_nearest_to(const DiPoint *target, DiPoint *p1, DiPoint *p2){
    int dp1x = target->x - p1->x; 
    int dp1y = target->y - p1->y;

    int dp2x = target->x - p2->x; 
    int dp2y = target->y - p2->y;

    if((dp1x * dp1x + dp1y * dp1y) > (dp2x * dp2x + dp2y * dp2y)){
        return p2;
    }
    else{
        return p1;
    }
}

DiColor *di_trash_can(void){
    static DiColor trash;
    return &trash;
}

static void blend_set_src(DiColor *dst, const DiColor *src){
    blend_set_src_color(dst, src);
    blend_set_src_alpha(dst, src);
}

static void blend_set_src_color(DiColor *dst, const DiColor *src){
    dst->r = src->r;
    dst->g = src->g;
    dst->b = src->b;
}

static void blend_set_src_alpha(DiColor *dst, const DiColor *src){
    dst->a = src->a;
}
