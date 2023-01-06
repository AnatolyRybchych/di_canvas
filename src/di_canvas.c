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
    assert(canvas != NULL);
    assert(filename != NULL);

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
    assert(canvas != NULL);
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

static void __draw_point(int x, int y, void *data){
    void **a = data;
    DiCanvas *canvas = a[0];
    DiColor *color = a[1];

    canvas->blend_func(&DI_PIXEL_SAFE(*canvas, x, y), color); 
}

void di_draw_line(DiCanvas *canvas, DiPoint p1, DiPoint p2, DiColor color){
    void *a[] = {
        [0] = canvas,
        [1] = &color,
    };
    
    di_enum_line_points(p1, p2, __draw_point, a);
}

//!!!dx cannot be 0
static void __draw_line(int x, int y, void *data){
    void **a = data;
    
    DiCanvas *canvas = a[0];
    DiColor *color = a[1];

    int *dx = a[2];
    int *dy = a[3];
    int *b = a[4];

    int end_y = *dy * x / *dx + *b;
    int inc = y > end_y ? -1 : 1;

    while (y != end_y){
        canvas->blend_func(&DI_PIXEL_SAFE(*canvas, x, y), color); 
        y += inc;
    }
    canvas->blend_func(&DI_PIXEL_SAFE(*canvas, x, y), color); 
}

void di_draw_triangle(DiCanvas *canvas, DiPoint p1, DiPoint p2, DiPoint p3, DiColor color){
    assert(canvas != NULL);
    if(p2.x < p1.x) DI_SWAP(p1, p2);
    if(p3.x < p2.x) DI_SWAP(p2, p3);
    if(p2.x < p1.x) DI_SWAP(p1, p2);

    int dx = p3.x - p1.x;
    if(dx == 0) return;
    
    int dy = p3.y - p1.y;
    int b = p1.y - p1.x * dy / dx;

    void *a[] = {
        [0] = canvas,
        [1] = &color,
        [2] = &dx,
        [3] = &dy,
        [4] = &b,
    };

    di_enum_line_points_unique_x(p1, p2, __draw_line, a);
    di_enum_line_points_unique_x(p2, p3, __draw_line, a);
}

void di_stroke_circle(DiCanvas *canvas, DiPoint p, uint32_t r, DiColor color){
    assert(canvas != NULL);

    //x**2 + y**2 = r**2
    //y**2 = r**2 - x**2

    int r_3_4 = (int)r*3/4;
    
    for (int circle_x = -r_3_4; circle_x <= r_3_4; circle_x++){
        int circle_y = sqrtf(r*r - circle_x*circle_x);
        int x = circle_x + p.x;
        int y1 = circle_y + p.y;
        int y2 = -circle_y + p.y;

        canvas->blend_func(&DI_PIXEL_SAFE(*canvas, x, y1), &color);
        canvas->blend_func(&DI_PIXEL_SAFE(*canvas, x, y2), &color);
    }

    for (int circle_y = -r_3_4; circle_y <= r_3_4; circle_y++){
        int circle_x = sqrtf(r*r - circle_y*circle_y);
        int y = circle_y + p.y;
        int x1 = circle_x + p.x;
        int x2 = -circle_x + p.x;

        canvas->blend_func(&DI_PIXEL_SAFE(*canvas, x1, y), &color);
        canvas->blend_func(&DI_PIXEL_SAFE(*canvas, x2, y), &color);
    }
}

void di_fill_circle(DiCanvas *canvas, DiPoint p, uint32_t r, DiColor color){
    assert(canvas != NULL);
    
    for (int circle_x = -r; circle_x <= (int)r; circle_x++){
        int circle_y = sqrtf(r*r - circle_x*circle_x);
        int x = circle_x + p.x;
        int y1 = circle_y + p.y;
        int y2 = -circle_y + p.y;
        
        while (y1 >= y2){
            canvas->blend_func(&DI_PIXEL_SAFE(*canvas, x, y1), &color);
            y1--;
        }
    }
}

DiPoint *di_nearest_to(const DiPoint *target, DiPoint *p1, DiPoint *p2){
    assert(target != NULL);
    assert(p1 != NULL);
    assert(p2 != NULL);

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

void di_enum_line_points(DiPoint p1, DiPoint p2, void (*enum_proc)(int x, int y, void *data), void *data){
    assert(enum_proc != NULL);

    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;

    if (dx < dy){
        di_enum_line_points_unique_y(p1, p2, enum_proc, data);
    }
    else{
        di_enum_line_points_unique_x(p1, p2, enum_proc, data);
    }
}

void di_enum_line_points_unique_x(DiPoint p1, DiPoint p2, void (*enum_proc)(int x, int y, void *data), void *data){
    assert(enum_proc != NULL);

    int dx = p2.x - p1.x;
    int sx = DI_SIGN(dx);
    int dy = p2.y - p1.y;
    
    int b = p1.y - p1.x * dy / dx;

    if(dx == 0){
        enum_proc(p1.x, p1.y, data);
        return;
    }

    do{
        enum_proc(p1.x, dy * p1.x / dx + b, data);
        p1.x += sx;
    } while(p1.x != p2.x);
}

void di_enum_line_points_unique_y(DiPoint p1, DiPoint p2, void (*enum_proc)(int x, int y, void *data), void *data){
    assert(enum_proc != NULL);

    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;
    int sy = DI_SIGN(dy);
    
    int b = p1.y - p1.x * dy / dx;

    if(dy == 0){
        enum_proc(p1.x, p1.y, data);
        return;
    }

    do{
        //y = k * x + b
        //x = (y - b) / k
        enum_proc((p1.y - b) * dx / dy, p1.y, data);
        p1.y += sy;
    } while(p1.y != p2.y);
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
