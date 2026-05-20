#include <errno.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct {
    uint32_t codepoint;
    int width;
    int height;
    int left;
    int top;
    int advance;
    int x;
    int y;
    unsigned char *bitmap;
} GlyphBitmap;

typedef struct {
    int width;
    int height;
} AtlasDims;

typedef struct {
    const char *font_path;
    const char *glyphs_path;
    const char *out_dir;
    int *sizes;
    size_t size_count;
} Options;

static void usage(const char *prog) {
    fprintf(stderr,
            "usage: %s --font FONT --glyphs GLYPHS --out-dir DIR --size N [--size N ...]\n",
            prog);
}

static int ensure_dir(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return S_ISDIR(st.st_mode) ? 0 : -1;
    }
    if (mkdir(path, 0755) == 0) {
        return 0;
    }
    return errno == EEXIST ? 0 : -1;
}

static int cmp_u32(const void *lhs, const void *rhs) {
    const uint32_t a = *(const uint32_t *)lhs;
    const uint32_t b = *(const uint32_t *)rhs;
    if (a < b) {
        return -1;
    }
    if (a > b) {
        return 1;
    }
    return 0;
}

static int cmp_glyph_height_desc(const void *lhs, const void *rhs) {
    const GlyphBitmap *a = *(GlyphBitmap * const *)lhs;
    const GlyphBitmap *b = *(GlyphBitmap * const *)rhs;
    if (a->height != b->height) {
        return b->height - a->height;
    }
    if (a->width != b->width) {
        return b->width - a->width;
    }
    if (a->codepoint < b->codepoint) {
        return -1;
    }
    if (a->codepoint > b->codepoint) {
        return 1;
    }
    return 0;
}

static int next_pow2(int value) {
    int out = 1;
    while (out < value) {
        out <<= 1;
    }
    return out;
}

static int parse_args(int argc, char **argv, Options *out) {
    memset(out, 0, sizeof(*out));
    out->sizes = (int *)calloc((size_t)argc, sizeof(int));
    if (out->sizes == NULL) {
        return -1;
    }

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--font") == 0 && i + 1 < argc) {
            out->font_path = argv[++i];
        } else if (strcmp(argv[i], "--glyphs") == 0 && i + 1 < argc) {
            out->glyphs_path = argv[++i];
        } else if (strcmp(argv[i], "--out-dir") == 0 && i + 1 < argc) {
            out->out_dir = argv[++i];
        } else if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            out->sizes[out->size_count++] = atoi(argv[++i]);
        } else {
            usage(argv[0]);
            return -1;
        }
    }

    if (out->font_path == NULL || out->glyphs_path == NULL || out->out_dir == NULL || out->size_count == 0) {
        usage(argv[0]);
        return -1;
    }
    return 0;
}

static int read_codepoints(const char *path, uint32_t **out_codes, size_t *out_count) {
    FILE *fp = fopen(path, "rb");
    char line[128];
    size_t cap = 1024;
    size_t count = 0;
    uint32_t *codes = (uint32_t *)malloc(cap * sizeof(uint32_t));
    if (fp == NULL || codes == NULL) {
        fclose(fp);
        free(codes);
        return -1;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *cursor = line;
        while (*cursor == ' ' || *cursor == '\t') {
            ++cursor;
        }
        if (*cursor == '#' || *cursor == '\n' || *cursor == '\r' || *cursor == '\0') {
            continue;
        }
        errno = 0;
        char *end = NULL;
        unsigned long value = strtoul(cursor, &end, 16);
        if (errno != 0 || end == cursor || value > 0x10FFFFul) {
            fclose(fp);
            free(codes);
            return -1;
        }
        if (count == cap) {
            cap *= 2;
            uint32_t *grown = (uint32_t *)realloc(codes, cap * sizeof(uint32_t));
            if (grown == NULL) {
                fclose(fp);
                free(codes);
                return -1;
            }
            codes = grown;
        }
        codes[count++] = (uint32_t)value;
    }

    fclose(fp);
    qsort(codes, count, sizeof(uint32_t), cmp_u32);

    size_t unique = 0;
    for (size_t i = 0; i < count; ++i) {
        if (unique == 0 || codes[i] != codes[unique - 1]) {
            codes[unique++] = codes[i];
        }
    }

    *out_codes = codes;
    *out_count = unique;
    return 0;
}

static void free_glyphs(GlyphBitmap *glyphs, size_t count) {
    if (glyphs == NULL) {
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        free(glyphs[i].bitmap);
    }
    free(glyphs);
}

static int load_glyphs(FT_Face face, const uint32_t *codes, size_t count, GlyphBitmap **out_glyphs,
                       int *out_line_height, int *out_baseline) {
    GlyphBitmap *glyphs = (GlyphBitmap *)calloc(count, sizeof(GlyphBitmap));
    if (glyphs == NULL) {
        return -1;
    }

    const int line_height = (int)((face->size->metrics.height + 32) >> 6);
    const int baseline = (int)((face->size->metrics.ascender + 32) >> 6);

    for (size_t i = 0; i < count; ++i) {
        GlyphBitmap *glyph = &glyphs[i];
        glyph->codepoint = codes[i];

        if (FT_Load_Char(face, codes[i], FT_LOAD_DEFAULT) != 0) {
            fprintf(stderr, "error: glyph U+%04X is missing from %s\n", codes[i], face->family_name);
            free_glyphs(glyphs, count);
            return -1;
        }
        if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL) != 0) {
            fprintf(stderr, "error: failed to render U+%04X\n", codes[i]);
            free_glyphs(glyphs, count);
            return -1;
        }

        FT_GlyphSlot slot = face->glyph;
        glyph->width = (int)slot->bitmap.width;
        glyph->height = (int)slot->bitmap.rows;
        glyph->left = (int)slot->bitmap_left;
        glyph->top = (int)slot->bitmap_top;
        glyph->advance = (int)((slot->advance.x + 32) >> 6);
        glyph->x = 0;
        glyph->y = 0;

        if (glyph->width == 0 || glyph->height == 0) {
            glyph->bitmap = NULL;
            continue;
        }

        size_t size = (size_t)glyph->width * (size_t)glyph->height;
        glyph->bitmap = (unsigned char *)malloc(size);
        if (glyph->bitmap == NULL) {
            free_glyphs(glyphs, count);
            return -1;
        }
        for (int row = 0; row < glyph->height; ++row) {
            memcpy(glyph->bitmap + (size_t)row * (size_t)glyph->width,
                   slot->bitmap.buffer + (size_t)row * (size_t)slot->bitmap.pitch,
                   (size_t)glyph->width);
        }
    }

    *out_glyphs = glyphs;
    *out_line_height = line_height > 0 ? line_height : (int)face->size->metrics.y_ppem;
    *out_baseline = baseline > 0 ? baseline : (int)face->size->metrics.y_ppem;
    return 0;
}

static int simulate_pack(GlyphBitmap * const *packed, size_t count, int atlas_w, int padding) {
    int x = padding;
    int y = padding;
    int row_h = 0;
    for (size_t i = 0; i < count; ++i) {
        GlyphBitmap *glyph = packed[i];
        if (glyph->width == 0 || glyph->height == 0) {
            continue;
        }
        if (x + glyph->width + padding > atlas_w) {
            x = padding;
            y += row_h;
            row_h = 0;
        }
        if (glyph->height + padding > row_h) {
            row_h = glyph->height + padding;
        }
        x += glyph->width + padding;
    }
    return y + row_h + padding;
}

static AtlasDims choose_dims(GlyphBitmap *glyphs, size_t count) {
    const int padding = 1;
    int max_w = 1;
    size_t area = 0;
    GlyphBitmap **packed = (GlyphBitmap **)malloc(count * sizeof(GlyphBitmap *));
    AtlasDims dims = {256, 256};

    for (size_t i = 0; i < count; ++i) {
        packed[i] = &glyphs[i];
        if (glyphs[i].width > max_w) {
            max_w = glyphs[i].width;
        }
        area += (size_t)(glyphs[i].width + padding) * (size_t)(glyphs[i].height + padding);
    }
    qsort(packed, count, sizeof(GlyphBitmap *), cmp_glyph_height_desc);

    int best_w = 0;
    int best_h = 0;
    long long best_area = 0;
    int start_w = next_pow2(max_w + padding * 2);
    if (start_w < 128) {
        start_w = 128;
    }

    const int approx = next_pow2((int)(sqrt((double)(area == 0 ? 1 : area)) * 1.15) + 1);
    if (approx > start_w) {
        start_w = approx;
    }

    for (int atlas_w = start_w; atlas_w <= 4096; atlas_w <<= 1) {
        int atlas_h = simulate_pack(packed, count, atlas_w, padding);
        long long packed_area = (long long)atlas_w * (long long)atlas_h;
        if (best_w == 0 || packed_area < best_area) {
            best_w = atlas_w;
            best_h = atlas_h;
            best_area = packed_area;
        }
        if (atlas_w >= 1024 && atlas_h <= atlas_w) {
            break;
        }
    }

    free(packed);
    dims.width = best_w > 0 ? best_w : 256;
    dims.height = best_h > 0 ? best_h : 256;
    return dims;
}

static void place_glyphs(GlyphBitmap *glyphs, size_t count, AtlasDims dims) {
    const int padding = 1;
    GlyphBitmap **packed = (GlyphBitmap **)malloc(count * sizeof(GlyphBitmap *));
    int x = padding;
    int y = padding;
    int row_h = 0;

    for (size_t i = 0; i < count; ++i) {
        packed[i] = &glyphs[i];
    }
    qsort(packed, count, sizeof(GlyphBitmap *), cmp_glyph_height_desc);

    for (size_t i = 0; i < count; ++i) {
        GlyphBitmap *glyph = packed[i];
        if (glyph->width == 0 || glyph->height == 0) {
            glyph->x = 0;
            glyph->y = 0;
            continue;
        }
        if (x + glyph->width + padding > dims.width) {
            x = padding;
            y += row_h;
            row_h = 0;
        }
        glyph->x = x;
        glyph->y = y;
        if (glyph->height + padding > row_h) {
            row_h = glyph->height + padding;
        }
        x += glyph->width + padding;
    }

    free(packed);
}

static int write_binary_atlas(const char *path, const GlyphBitmap *glyphs, size_t count, AtlasDims dims) {
    const size_t total = (size_t)dims.width * (size_t)dims.height;
    unsigned char *atlas = (unsigned char *)calloc(total, 1);
    FILE *fp = NULL;
    if (atlas == NULL) {
        return -1;
    }

    for (size_t i = 0; i < count; ++i) {
        const GlyphBitmap *glyph = &glyphs[i];
        if (glyph->bitmap == NULL) {
            continue;
        }
        for (int row = 0; row < glyph->height; ++row) {
            unsigned char *dst = atlas + (size_t)(glyph->y + row) * (size_t)dims.width + (size_t)glyph->x;
            const unsigned char *src = glyph->bitmap + (size_t)row * (size_t)glyph->width;
            memcpy(dst, src, (size_t)glyph->width);
        }
    }

    fp = fopen(path, "wb");
    if (fp == NULL) {
        free(atlas);
        return -1;
    }
    if (fwrite(atlas, 1, total, fp) != total) {
        fclose(fp);
        free(atlas);
        return -1;
    }
    fclose(fp);
    free(atlas);
    return 0;
}

static int write_fnt(const char *path, const char *asset_name, const GlyphBitmap *glyphs, size_t count,
                     AtlasDims dims, int pixel_size, int line_height, int baseline) {
    FILE *fp = fopen(path, "wb");
    if (fp == NULL) {
        return -1;
    }

    fprintf(fp,
            "info face=\"WenQuanYi Micro Hei Demo\" size=%d bold=0 italic=0 charset=\"\" unicode=1 stretchH=100 smooth=1 aa=1 padding=0,0,0,0 spacing=1,1\n",
            pixel_size);
    fprintf(fp,
            "common lineHeight=%d base=%d scaleW=%d scaleH=%d pages=1 packed=0\n",
            line_height, baseline, dims.width, dims.height);
    fprintf(fp, "page id=0 file=\"%s\"\n", asset_name);
    fprintf(fp, "chars count=%zu\n", count);
    for (size_t i = 0; i < count; ++i) {
        const GlyphBitmap *glyph = &glyphs[i];
        const int xoffset = glyph->left;
        const int yoffset = baseline - glyph->top;
        fprintf(fp,
                "char id=%u x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=0 chnl=0\n",
                glyph->codepoint, glyph->x, glyph->y, glyph->width, glyph->height,
                xoffset, yoffset, glyph->advance);
    }
    fprintf(fp, "kernings count=0\n");

    fclose(fp);
    return 0;
}

static int generate_size(FT_Library library, const Options *opts, const uint32_t *codes, size_t code_count, int pixel_size) {
    FT_Face face = NULL;
    GlyphBitmap *glyphs = NULL;
    AtlasDims dims;
    char atlas_path[512];
    char fnt_path[512];
    char asset_name[128];
    int line_height = 0;
    int baseline = 0;

    if (FT_New_Face(library, opts->font_path, 0, &face) != 0) {
        fprintf(stderr, "error: failed to open font %s\n", opts->font_path);
        return -1;
    }
    if (FT_Set_Pixel_Sizes(face, 0, (FT_UInt)pixel_size) != 0) {
        FT_Done_Face(face);
        fprintf(stderr, "error: failed to set pixel size %d\n", pixel_size);
        return -1;
    }
    if (load_glyphs(face, codes, code_count, &glyphs, &line_height, &baseline) != 0) {
        FT_Done_Face(face);
        return -1;
    }

    dims = choose_dims(glyphs, code_count);
    place_glyphs(glyphs, code_count, dims);

    snprintf(asset_name, sizeof(asset_name), "wqy_microhei_demo_%d.a8", pixel_size);
    snprintf(atlas_path, sizeof(atlas_path), "%s/%s", opts->out_dir, asset_name);
    snprintf(fnt_path, sizeof(fnt_path), "%s/wqy_microhei_demo_%d.fnt", opts->out_dir, pixel_size);

    if (write_binary_atlas(atlas_path, glyphs, code_count, dims) != 0) {
        fprintf(stderr, "error: failed to write %s\n", atlas_path);
        free_glyphs(glyphs, code_count);
        FT_Done_Face(face);
        return -1;
    }
    if (write_fnt(fnt_path, asset_name, glyphs, code_count, dims, pixel_size, line_height, baseline) != 0) {
        fprintf(stderr, "error: failed to write %s\n", fnt_path);
        free_glyphs(glyphs, code_count);
        FT_Done_Face(face);
        return -1;
    }

    printf("generated %s and %s (%zu glyphs, %dx%d)\n",
           atlas_path, fnt_path, code_count, dims.width, dims.height);

    free_glyphs(glyphs, code_count);
    FT_Done_Face(face);
    return 0;
}

int main(int argc, char **argv) {
    Options opts;
    FT_Library library = NULL;
    uint32_t *codes = NULL;
    size_t code_count = 0;

    if (parse_args(argc, argv, &opts) != 0) {
        free(opts.sizes);
        return 1;
    }
    if (ensure_dir(opts.out_dir) != 0) {
        fprintf(stderr, "error: failed to create %s\n", opts.out_dir);
        free(opts.sizes);
        return 1;
    }
    if (read_codepoints(opts.glyphs_path, &codes, &code_count) != 0) {
        fprintf(stderr, "error: failed to read glyph list %s\n", opts.glyphs_path);
        free(opts.sizes);
        return 1;
    }
    if (FT_Init_FreeType(&library) != 0) {
        fprintf(stderr, "error: failed to initialize FreeType\n");
        free(codes);
        free(opts.sizes);
        return 1;
    }

    for (size_t i = 0; i < opts.size_count; ++i) {
        if (generate_size(library, &opts, codes, code_count, opts.sizes[i]) != 0) {
            FT_Done_FreeType(library);
            free(codes);
            free(opts.sizes);
            return 1;
        }
    }

    FT_Done_FreeType(library);
    free(codes);
    free(opts.sizes);
    return 0;
}
