#include "libs.h"
#include "tui.h"
#include "img_struct.h"
#include <zlib.h>

// Deklaracje funkcji pomocniczych z innych plików
void save_image_ppm(Image_data* img);
void free_image(Image_data* img);
Image_data* create_empty(uint16_t max_pixel_val, PPM_TYPE img_type, size_t width, size_t height, char* name);

// Macierz przejścia RGB -> YCbCr
static const float m[3][3] = {
        {0.299f, 0.587f, 0.114f},
        {-0.169f, -0.331f, 0.500f},
        {0.500f, -0.419f, -0.081f}
};

// Macierz odwrotna (YCbCr -> RGB)
// Wartości wyznaczone przez wolframalpha
static const float rev_m[3][3] = {
    {1.0f, -0.000926744840485595f, 1.401686760243916f},
    {1.0f, -0.3436953844721575f, -0.7141690399515892f},
    {1.0f, 1.7721604157233477f, 0.000990220514491491f}
};

// Mapa przejść ZigZag dla bloku 8x8
static const int ZZ[64][2] = {
    {0,0}, {1,0}, {0,1}, {0,2}, {1,1}, {2,0}, {3,0}, {2,1},
    {1,2}, {0,3}, {0,4}, {1,3}, {2,2}, {3,1}, {4,0}, {5,0},
    {4,1}, {3,2}, {2,3}, {1,4}, {0,5}, {0,6}, {1,5}, {2,4},
    {3,3}, {4,2}, {5,1}, {6,0}, {7,0}, {6,1}, {5,2}, {4,3},
    {3,4}, {2,5}, {1,6}, {0,7}, {1,7}, {2,6}, {3,5}, {4,4},
    {5,3}, {6,2}, {7,1}, {7,2}, {6,3}, {5,4}, {4,5}, {3,6},
    {2,7}, {3,7}, {4,6}, {5,5}, {6,4}, {7,3}, {7,4}, {6,5},
    {5,6}, {4,7}, {5,7}, {6,6}, {7,5}, {7,6}, {6,7}, {7,7}
};

// Funkcja pomocnicza: ograniczenie wartości float do uint8_t (8 bitów) 
static uint8_t clamp(float value){

    if(value < 0.0f)
        return 0;

    if(value > 255.0f)
        return 255;

    return (uint8_t)(value + 0.5f);

}

// Funkcja pomocnicza: redukcja rozdzielczości
uint8_t* downsample(uint8_t* src, int src_w, int src_h, int factor, int* out_w, int* out_h){

    if(!src) 
        return NULL;

    *out_w = src_w / factor;
    *out_h = src_h / factor;
    
    uint8_t* dst = malloc((*out_w) * (*out_h));
    if(!dst){
        print_error("Malloc spadl z rowerka: downsample array allocation");
        return NULL;
    }

    #pragma omp parallel for collapse(2)
    for(int y=0; y < *out_h; y++){
        for(int x=0; x < *out_w; x++){
            dst[y * (*out_w) + x] = src[(y * factor) * src_w + (x * factor)];
        }
    }

    return dst;

}

// Funkcja pomocnicza: zwiększanie rozdzielczości
uint8_t* upsample(uint8_t* src, int src_w, int src_h, int factor){

    if(!src)
        return NULL;

    int out_w = src_w * factor;
    int out_h = src_h * factor;
    
    uint8_t* dst = malloc(out_w * out_h);
    if(!dst){
        print_error("Malloc spadl z rowerka: upsample array allocation");
        return NULL;
    }

    #pragma omp parallel for collapse(2)
    for(int y=0; y < out_h; y++){
        for(int x=0; x < out_w; x++){
            dst[y * out_w + x] = src[(y / factor) * src_w + (x / factor)];
        }
    }

    return dst;

}

// Implemantacja algorytmu ZigZag
void apply_zigzag(uint8_t* src, int w, int h, uint8_t* dst){

    if(!src || !dst)
        return;

    int blocks_x = w / 8;
    int blocks_y = h / 8;

    #pragma omp parallel for collapse(2)
    for(int by_idx = 0; by_idx < blocks_y; by_idx++){
        for(int bx_idx = 0; bx_idx < blocks_x; bx_idx++){

            int by = by_idx * 8;
            int bx = bx_idx * 8;
            int base_idx = (by_idx * blocks_x + bx_idx) * 64;
            
            for(int i = 0; i < 64; i++){

                int zx = ZZ[i][0];
                int zy = ZZ[i][1];
                dst[base_idx + i] = src[(by + zy) * w + (bx + zx)];

            }

        }

    }

}

// Odwrócenie ZigZag
void inverse_zigzag(uint8_t* src, int w, int h, uint8_t* dst){

    if(!src || !dst)
        return;

    int blocks_x = w / 8;
    int blocks_y = h / 8;

    #pragma omp parallel for collapse(2)
    for (int by_idx = 0; by_idx < blocks_y; by_idx++){
        for (int bx_idx = 0; bx_idx < blocks_x; bx_idx++){

            int by = by_idx * 8;
            int bx = bx_idx * 8;
            int base_idx = (by_idx * blocks_x + bx_idx) * 64;
            
            for(int i = 0; i < 64; i++){
                int zx = ZZ[i][0];
                int zy = ZZ[i][1];
                dst[(by + zy) * w + (bx + zx)] = src[base_idx + i];
            }

        }

    }

}

// Główna funkcja procesująca JPEG
void process_jpeg_pipeline(Image_data* input, int subsample_factor){
    printf("\n\t--- Test algorytmu ze wspolczynnikiem probkowania: %d ---\n", subsample_factor);

    size_t w = (input->width + 31) & ~31;
    size_t h = (input->height + 31) & ~31;

    // 1. Inicjalizacja wszystkich wskaźników na NULL
    uint8_t *Y = NULL, *Cb = NULL, *Cr = NULL;
    uint8_t *Cb_sub = NULL, *Cr_sub = NULL;
    uint8_t *Y_zz = NULL, *Cb_zz = NULL, *Cr_zz = NULL;
    uint8_t *merged_zz = NULL, *comp_data = NULL, *decomp_merged = NULL;
    uint8_t *Y_dec = NULL, *Cb_sub_dec = NULL, *Cr_sub_dec = NULL;
    uint8_t *Cb_dec = NULL, *Cr_dec = NULL;
    Image_data *out = NULL;

    Y = malloc(w * h);
    Cb = malloc(w * h);
    Cr = malloc(w * h);
    if(!Y || !Cb || !Cr){
        print_error("Malloc spadl z rowerka: Y, Cb, Cr arrays");
        goto cleanup;
    }

    // KROK 1: Konwersja barw
    #pragma omp parallel for collapse(2)
    for(size_t y = 0; y < h; y++){
        for(size_t x = 0; x < w; x++){

            size_t px = x < input->width ? x : input->width - 1;
            size_t py = y < input->height ? y : input->height - 1;
            Pixel_data p = input->pixels[py * input->width + px];

            Y[y*w+x]  = clamp(m[0][0]*p.R + m[0][1]*p.G + m[0][2]*p.B);
            Cb[y*w+x] = clamp(m[1][0]*p.R + m[1][1]*p.G + m[1][2]*p.B + 128.0f);
            Cr[y*w+x] = clamp(m[2][0]*p.R + m[2][1]*p.G + m[2][2]*p.B + 128.0f);

        }

    }

    // KROK 2: Przeskalowanie w dół
    int cw, ch;
    Cb_sub = downsample(Cb, w, h, subsample_factor, &cw, &ch);
    Cr_sub = downsample(Cr, w, h, subsample_factor, &cw, &ch);
    if(!Cb_sub || !Cr_sub){
        print_error("Przerwanie z powodu bledu alokacji w downsample");
        goto cleanup;
    }

    // KROK 3 & 7: Podział na bloki i ZigZag
    Y_zz = malloc(w * h);
    Cb_zz = malloc(cw * ch);
    Cr_zz = malloc(cw * ch);
    if(!Y_zz || !Cb_zz || !Cr_zz){
        print_error("Malloc spadl z rowerka: ZigZag arrays");
        goto cleanup;
    }
    
    apply_zigzag(Y, w, h, Y_zz);
    apply_zigzag(Cb_sub, cw, ch, Cb_zz);
    apply_zigzag(Cr_sub, cw, ch, Cr_zz);

    // KROK 8: Kompresja zlib
    size_t total_zz_size = (w * h) + 2 * (cw * ch);
    merged_zz = malloc(total_zz_size);
    if(!merged_zz){
        print_error("Malloc spadl z rowerka: merged_zz array");
        goto cleanup;
    }

    memcpy(merged_zz, Y_zz, w * h);
    memcpy(merged_zz + w*h, Cb_zz, cw*ch);
    memcpy(merged_zz + w*h + cw*ch, Cr_zz, cw*ch);

    uLongf comp_size = compressBound(total_zz_size);
    comp_data = malloc(comp_size);
    if(!comp_data){
        print_error("Malloc spadl z rowerka: comp_data array");
        goto cleanup;
    }

    if(compress(comp_data, &comp_size, merged_zz, total_zz_size) != Z_OK){
        print_error("Zlib compression spadl z rowerka");
        goto cleanup;
    }

    printf("\tRozmiar nieskompresowany (z paddingiem): %lu bajtow\n", total_zz_size);
    printf("\tRozmiar po kompresji ZLIB (Krok 8):      %lu bajtow\n", comp_size);

    // =================== DEKOMPRESJA =================
    decomp_merged = malloc(total_zz_size);
    if(!decomp_merged){
        print_error("Malloc spadl z rowerka: decomp_merged array");
        goto cleanup;
    }

    uLongf decomp_size = total_zz_size;
    if(uncompress(decomp_merged, &decomp_size, comp_data, comp_size) != Z_OK){
        print_error("Zlib decompression spadl z rowerka");
        goto cleanup;
    }

    // Odwrotność 7 i 3: Inverse ZigZag
    Y_dec = malloc(w * h);
    Cb_sub_dec = malloc(cw * ch);
    Cr_sub_dec = malloc(cw * ch);
    if(!Y_dec || !Cb_sub_dec || !Cr_sub_dec){
        print_error("Malloc spadl z rowerka: Inverse ZigZag arrays");
        goto cleanup;
    }
    
    inverse_zigzag(decomp_merged, w, h, Y_dec);
    inverse_zigzag(decomp_merged + w*h, cw, ch, Cb_sub_dec);
    inverse_zigzag(decomp_merged + w*h + cw*ch, cw, ch, Cr_sub_dec);

    // Odwrotność 2: Upsampling
    Cb_dec = upsample(Cb_sub_dec, cw, ch, subsample_factor);
    Cr_dec = upsample(Cr_sub_dec, cw, ch, subsample_factor);
    if(!Cb_dec || !Cr_dec){
        print_error("Przerwanie z powodu bledu alokacji w upsample");
        goto cleanup;
    }

    // Odwrotność 1: Konwersja YCbCr -> RGB
    char out_name[32];
    sprintf(out_name, "w4_test_factor_%d.ppm", subsample_factor);
    out = create_empty(input->max_pixel_val, TYPE_P6, input->width, input->height, out_name);
    if(!out){
        print_error("Nie udalo sie utworzyc wynikowego Image_data struct");
        goto cleanup;
    }

    #pragma omp parallel for collapse(2)
    for(size_t y = 0; y < input->height; y++){
        for(size_t x = 0; x < input->width; x++){

            float y_val = Y_dec[y*w+x];
            float cb_val = Cb_dec[y*w+x] - 128.0f;
            float cr_val = Cr_dec[y*w+x] - 128.0f;

            float r = rev_m[0][0]*y_val + rev_m[0][1]*cb_val + rev_m[0][2]*cr_val;
            float g = rev_m[1][0]*y_val + rev_m[1][1]*cb_val + rev_m[1][2]*cr_val;
            float b = rev_m[2][0]*y_val + rev_m[2][1]*cb_val + rev_m[2][2]*cr_val;

            out->pixels[y * input->width + x].R = clamp(r);
            out->pixels[y * input->width + x].G = clamp(g);
            out->pixels[y * input->width + x].B = clamp(b);

        }

    }

    save_image_ppm(out);

cleanup:
    // Czyszczenie pamięci
    if(Y) free(Y);
    if(Cb) free(Cb);
    if(Cr) free(Cr);
    if(Cb_sub) free(Cb_sub);
    if(Cr_sub) free(Cr_sub);
    if(Y_zz) free(Y_zz);
    if(Cb_zz) free(Cb_zz);
    if(Cr_zz) free(Cr_zz);
    if(merged_zz) free(merged_zz);
    if(comp_data) free(comp_data);
    if(decomp_merged) free(decomp_merged);
    if(Y_dec) free(Y_dec);
    if(Cb_sub_dec) free(Cb_sub_dec);
    if(Cr_sub_dec) free(Cr_sub_dec);
    if(Cb_dec) free(Cb_dec);
    if(Cr_dec) free(Cr_dec);
    if(out) free_image(out);

}
