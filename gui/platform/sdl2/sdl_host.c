#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <stdint.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct UyaGuiSimGles2Fns {
    void (*ActiveTexture)(GLenum texture);
    void (*AttachShader)(GLuint program, GLuint shader);
    void (*BindBuffer)(GLenum target, GLuint buffer);
    void (*BindTexture)(GLenum target, GLuint texture);
    void (*BufferData)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
    void (*Clear)(GLbitfield mask);
    void (*ClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void (*CompileShader)(GLuint shader);
    GLuint (*CreateProgram)(void);
    GLuint (*CreateShader)(GLenum type);
    void (*DeleteBuffers)(GLsizei n, const GLuint *buffers);
    void (*DeleteProgram)(GLuint program);
    void (*DeleteShader)(GLuint shader);
    void (*DeleteTextures)(GLsizei n, const GLuint *textures);
    void (*DisableVertexAttribArray)(GLuint index);
    void (*DrawArrays)(GLenum mode, GLint first, GLsizei count);
    void (*EnableVertexAttribArray)(GLuint index);
    void (*GenBuffers)(GLsizei n, GLuint *buffers);
    void (*GenTextures)(GLsizei n, GLuint *textures);
    GLint (*GetAttribLocation)(GLuint program, const GLchar *name);
    void (*GetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    void (*GetProgramiv)(GLuint program, GLenum pname, GLint *params);
    void (*GetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    void (*GetShaderiv)(GLuint shader, GLenum pname, GLint *params);
    GLint (*GetUniformLocation)(GLuint program, const GLchar *name);
    void (*LinkProgram)(GLuint program);
    void (*PixelStorei)(GLenum pname, GLint param);
    void (*ShaderSource)(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
    void (*TexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
    void (*TexParameteri)(GLenum target, GLenum pname, GLint param);
    void (*TexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
    void (*Uniform1i)(GLint location, GLint v0);
    void (*UseProgram)(GLuint program);
    void (*VertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
    void (*Viewport)(GLint x, GLint y, GLsizei width, GLsizei height);
} UyaGuiSimGles2Fns;

enum {
    UYA_GUI_SIM_GPU_AUTO = 0,
    UYA_GUI_SIM_GPU_SOFTWARE = 1,
    UYA_GUI_SIM_GPU_GLES2 = 2,
};

enum {
    UYA_GUI_SIM_PRESENT_SOFTWARE = 1,
    UYA_GUI_SIM_PRESENT_GLES2 = 2,
};

#define UYA_GUI_SIM_DIRTY_OVERLAY_MAX_RECTS 32

typedef struct UyaGuiSimDisplay {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_GLContext gl_context;
    UyaGuiSimGles2Fns gl;
    GLuint gl_program;
    GLuint gl_texture;
    GLuint gl_vbo;
    GLint gl_attr_pos;
    GLint gl_attr_uv;
    GLint gl_uniform_tex;
    uint8_t *gl_rgba_pixels;
    size_t gl_rgba_bytes;
    int width;
    int height;
    int scale;
    int fullscreen;
    int present_kind;
    const uint8_t *pending_pixels;
    int pending_pitch;
    int pending_width;
    int pending_height;
    int pending_format_tag;
    int pending_present;
    int dirty_overlay_enabled;
    int dirty_overlay_full;
    SDL_Rect dirty_overlay_rects[UYA_GUI_SIM_DIRTY_OVERLAY_MAX_RECTS];
    int dirty_overlay_rect_count;
    uint8_t *overlay_pixels;
    size_t overlay_bytes;
    int needs_refresh;
} UyaGuiSimDisplay;

typedef struct SdlHostEvent {
    uint8_t kind;
    int16_t x;
    int16_t y;
    int32_t value;
    uint16_t key_code;
    uint16_t reserved;
} SdlHostEvent;

enum {
    SDL_EVT_NONE = 0,
    SDL_EVT_QUIT = 1,
    SDL_EVT_POINTER_MOVE = 2,
    SDL_EVT_POINTER_DOWN = 3,
    SDL_EVT_POINTER_UP = 4,
    SDL_EVT_KEY_DOWN = 5,
    SDL_EVT_KEY_UP = 6,
    SDL_EVT_WHEEL = 7,
};

static int g_sdl_refcount = 0;
static UyaGuiSimDisplay *g_active_display = NULL;
static char g_last_error[256] = {0};
static SDL_Event g_pending_event;
static int g_has_pending_event = 0;
static void *(*g_host_malloc_fn)(size_t) = NULL;
static void *(*g_host_calloc_fn)(size_t, size_t) = NULL;
static void *(*g_host_realloc_fn)(void *, size_t) = NULL;
static void (*g_host_free_fn)(void *) = NULL;

static void uya_gui_sim_init_host_allocators(void) {
    if (g_host_malloc_fn != NULL && g_host_calloc_fn != NULL && g_host_realloc_fn != NULL && g_host_free_fn != NULL) {
        return;
    }

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
    g_host_malloc_fn = (void *(*)(size_t))dlsym(RTLD_NEXT, "malloc");
    g_host_calloc_fn = (void *(*)(size_t, size_t))dlsym(RTLD_NEXT, "calloc");
    g_host_realloc_fn = (void *(*)(void *, size_t))dlsym(RTLD_NEXT, "realloc");
    g_host_free_fn = (void (*)(void *))dlsym(RTLD_NEXT, "free");
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
}

static void *uya_gui_sim_host_malloc(size_t size) {
    uya_gui_sim_init_host_allocators();
    return g_host_malloc_fn != NULL ? g_host_malloc_fn(size) : NULL;
}

static void *uya_gui_sim_host_calloc(size_t nmemb, size_t size) {
    uya_gui_sim_init_host_allocators();
    return g_host_calloc_fn != NULL ? g_host_calloc_fn(nmemb, size) : NULL;
}

static void *uya_gui_sim_host_realloc(void *ptr, size_t size) {
    uya_gui_sim_init_host_allocators();
    return g_host_realloc_fn != NULL ? g_host_realloc_fn(ptr, size) : NULL;
}

static void uya_gui_sim_host_free(void *ptr) {
    uya_gui_sim_init_host_allocators();
    if (g_host_free_fn != NULL) {
        g_host_free_fn(ptr);
    }
}

static void uya_gui_sim_set_error(const char *message) {
    if (message == NULL || message[0] == '\0') {
        message = "unknown SDL2 error";
    }
    (void)snprintf(g_last_error, sizeof(g_last_error), "%s", message);
}

static const char *uya_gui_sim_present_name(const UyaGuiSimDisplay *display) {
    if (display != NULL && display->present_kind == UYA_GUI_SIM_PRESENT_GLES2) {
        return "gles2";
    }
    return "software";
}

static SDL_Window *uya_gui_sim_create_window(int width, int height, int scale, int fullscreen, const uint8_t *title, uint32_t extra_flags) {
    uint32_t window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | extra_flags;
    if (fullscreen != 0) {
        window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    } else {
        window_flags |= SDL_WINDOW_RESIZABLE;
    }
    return SDL_CreateWindow(
        title != NULL ? (const char *)title : "UyaGUI Linux Simulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width * (scale > 0 ? scale : 1),
        height * (scale > 0 ? scale : 1),
        window_flags
    );
}

static void uya_gui_sim_destroy_renderer_resources(UyaGuiSimDisplay *display) {
    if (display->texture != NULL) {
        SDL_DestroyTexture(display->texture);
        display->texture = NULL;
    }
    if (display->renderer != NULL) {
        SDL_DestroyRenderer(display->renderer);
        display->renderer = NULL;
    }
}

static void uya_gui_sim_destroy_overlay_resources(UyaGuiSimDisplay *display) {
    if (display == NULL) {
        return;
    }
    if (display->overlay_pixels != NULL) {
        uya_gui_sim_host_free(display->overlay_pixels);
        display->overlay_pixels = NULL;
    }
    display->overlay_bytes = 0u;
    display->dirty_overlay_rect_count = 0;
    display->dirty_overlay_full = 0;
}

static void uya_gui_sim_destroy_gles2_resources(UyaGuiSimDisplay *display) {
    if (display == NULL) {
        return;
    }
    if (display->gl_context != NULL) {
        if (display->window != NULL) {
            (void)SDL_GL_MakeCurrent(display->window, display->gl_context);
        }
        if (display->gl.DeleteBuffers != NULL && display->gl_vbo != 0u) {
            display->gl.DeleteBuffers(1, &display->gl_vbo);
        }
        if (display->gl.DeleteTextures != NULL && display->gl_texture != 0u) {
            display->gl.DeleteTextures(1, &display->gl_texture);
        }
        if (display->gl.DeleteProgram != NULL && display->gl_program != 0u) {
            display->gl.DeleteProgram(display->gl_program);
        }
        SDL_GL_DeleteContext(display->gl_context);
    }
    if (display->gl_rgba_pixels != NULL) {
        uya_gui_sim_host_free(display->gl_rgba_pixels);
        display->gl_rgba_pixels = NULL;
    }
    display->gl_rgba_bytes = 0u;
    display->gl_context = NULL;
    display->gl_program = 0u;
    display->gl_texture = 0u;
    display->gl_vbo = 0u;
    display->gl_attr_pos = -1;
    display->gl_attr_uv = -1;
    display->gl_uniform_tex = -1;
    (void)memset(&display->gl, 0, sizeof(display->gl));
}

static void uya_gui_sim_destroy_display_resources(UyaGuiSimDisplay *display) {
    if (display == NULL) {
        return;
    }
    uya_gui_sim_destroy_overlay_resources(display);
    uya_gui_sim_destroy_renderer_resources(display);
    uya_gui_sim_destroy_gles2_resources(display);
    if (display->window != NULL) {
        SDL_DestroyWindow(display->window);
        display->window = NULL;
    }
    display->present_kind = UYA_GUI_SIM_PRESENT_SOFTWARE;
}

static int uya_gui_sim_overlay_ensure_capacity(UyaGuiSimDisplay *display, int32_t width, int32_t height) {
    const size_t needed = (size_t)width * (size_t)height * 4u;
    if (display->overlay_bytes >= needed && display->overlay_pixels != NULL) {
        return 1;
    }
    uint8_t *resized = (uint8_t *)uya_gui_sim_host_realloc(display->overlay_pixels, needed);
    if (resized == NULL) {
        uya_gui_sim_set_error("overlay realloc failed");
        return 0;
    }
    display->overlay_pixels = resized;
    display->overlay_bytes = needed;
    return 1;
}

static void uya_gui_sim_overlay_copy_argb8888(UyaGuiSimDisplay *display, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height) {
    int32_t y = 0;
    while (y < height) {
        const uint8_t *src_row = pixels + (size_t)y * (size_t)pitch;
        uint8_t *dst_row = display->overlay_pixels + (size_t)y * (size_t)width * 4u;
        (void)memcpy(dst_row, src_row, (size_t)width * 4u);
        y += 1;
    }
}

static void uya_gui_sim_overlay_draw_pixel_argb8888(uint8_t *pixels, int32_t width, int32_t height, int32_t x, int32_t y) {
    if (pixels == NULL || x < 0 || y < 0 || x >= width || y >= height) {
        return;
    }
    uint8_t *dst = pixels + ((size_t)y * (size_t)width + (size_t)x) * 4u;
    dst[0] = 255u;
    dst[1] = 255u;
    dst[2] = 0u;
    dst[3] = 0u;
}

static void uya_gui_sim_overlay_draw_rect_argb8888(uint8_t *pixels, int32_t width, int32_t height, SDL_Rect rect) {
    int32_t x0 = rect.x;
    int32_t y0 = rect.y;
    int32_t x1 = rect.x + rect.w - 1;
    int32_t y1 = rect.y + rect.h - 1;
    int32_t x = 0;
    int32_t y = 0;

    if (rect.w <= 0 || rect.h <= 0) {
        return;
    }

    if (x0 < 0) {
        x0 = 0;
    }
    if (y0 < 0) {
        y0 = 0;
    }
    if (x1 >= width) {
        x1 = width - 1;
    }
    if (y1 >= height) {
        y1 = height - 1;
    }
    if (x0 > x1 || y0 > y1) {
        return;
    }

    x = x0;
    while (x <= x1) {
        uya_gui_sim_overlay_draw_pixel_argb8888(pixels, width, height, x, y0);
        uya_gui_sim_overlay_draw_pixel_argb8888(pixels, width, height, x, y1);
        x += 1;
    }

    y = y0;
    while (y <= y1) {
        uya_gui_sim_overlay_draw_pixel_argb8888(pixels, width, height, x0, y);
        uya_gui_sim_overlay_draw_pixel_argb8888(pixels, width, height, x1, y);
        y += 1;
    }
}

static const uint8_t *uya_gui_sim_overlay_prepare(UyaGuiSimDisplay *display, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height,
                                                  int32_t *out_pitch) {
    int32_t i = 0;
    if (out_pitch != NULL) {
        *out_pitch = pitch;
    }
    if (display == NULL || pixels == NULL || !display->dirty_overlay_enabled) {
        return pixels;
    }
    if (!uya_gui_sim_overlay_ensure_capacity(display, width, height)) {
        return pixels;
    }

    uya_gui_sim_overlay_copy_argb8888(display, pixels, pitch, width, height);
    if (display->dirty_overlay_full || display->dirty_overlay_rect_count <= 0) {
        SDL_Rect full = { 0, 0, width, height };
        uya_gui_sim_overlay_draw_rect_argb8888(display->overlay_pixels, width, height, full);
    } else {
        i = 0;
        while (i < display->dirty_overlay_rect_count) {
            uya_gui_sim_overlay_draw_rect_argb8888(display->overlay_pixels, width, height, display->dirty_overlay_rects[i]);
            i += 1;
        }
    }

    if (out_pitch != NULL) {
        *out_pitch = width * 4;
    }
    return display->overlay_pixels;
}

static int32_t uya_gui_sim_present_software(UyaGuiSimDisplay *display, const uint8_t *pixels, int32_t pitch) {
    if (SDL_UpdateTexture(display->texture, NULL, pixels, pitch) != 0) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }
    if (SDL_RenderClear(display->renderer) != 0) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }
    if (SDL_RenderCopy(display->renderer, display->texture, NULL, NULL) != 0) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }
    SDL_RenderPresent(display->renderer);
    display->needs_refresh = 0;
    return 1;
}

static void uya_gui_sim_convert_argb_to_rgba(uint8_t *dst, const uint8_t *src, int pitch, int width, int height) {
    int y = 0;
    while (y < height) {
        const uint8_t *src_row = src + (size_t)y * (size_t)pitch;
        uint8_t *dst_row = dst + (size_t)y * (size_t)width * 4u;
        int x = 0;
        while (x < width) {
            const uint8_t a = src_row[x * 4 + 0];
            const uint8_t r = src_row[x * 4 + 1];
            const uint8_t g = src_row[x * 4 + 2];
            const uint8_t b = src_row[x * 4 + 3];
            dst_row[x * 4 + 0] = r;
            dst_row[x * 4 + 1] = g;
            dst_row[x * 4 + 2] = b;
            dst_row[x * 4 + 3] = a;
            x += 1;
        }
        y += 1;
    }
}

static int uya_gui_sim_load_gles2_proc(void **slot, const char *name) {
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
    *slot = SDL_GL_GetProcAddress(name);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
    if (*slot == NULL) {
        uya_gui_sim_set_error(name);
        return 0;
    }
    return 1;
}

static int uya_gui_sim_load_gles2_functions(UyaGuiSimDisplay *display) {
    if (!uya_gui_sim_load_gles2_proc((void **)&display->gl.ActiveTexture, "glActiveTexture")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.AttachShader, "glAttachShader")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.BindBuffer, "glBindBuffer")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.BindTexture, "glBindTexture")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.BufferData, "glBufferData")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.Clear, "glClear")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.ClearColor, "glClearColor")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.CompileShader, "glCompileShader")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.CreateProgram, "glCreateProgram")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.CreateShader, "glCreateShader")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.DeleteBuffers, "glDeleteBuffers")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.DeleteProgram, "glDeleteProgram")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.DeleteShader, "glDeleteShader")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.DeleteTextures, "glDeleteTextures")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.DisableVertexAttribArray, "glDisableVertexAttribArray")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.DrawArrays, "glDrawArrays")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.EnableVertexAttribArray, "glEnableVertexAttribArray")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.GenBuffers, "glGenBuffers")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.GenTextures, "glGenTextures")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.GetAttribLocation, "glGetAttribLocation")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.GetProgramInfoLog, "glGetProgramInfoLog")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.GetProgramiv, "glGetProgramiv")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.GetShaderInfoLog, "glGetShaderInfoLog")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.GetShaderiv, "glGetShaderiv")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.GetUniformLocation, "glGetUniformLocation")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.LinkProgram, "glLinkProgram")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.PixelStorei, "glPixelStorei")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.ShaderSource, "glShaderSource")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.TexImage2D, "glTexImage2D")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.TexParameteri, "glTexParameteri")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.TexSubImage2D, "glTexSubImage2D")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.Uniform1i, "glUniform1i")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.UseProgram, "glUseProgram")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.VertexAttribPointer, "glVertexAttribPointer")
        || !uya_gui_sim_load_gles2_proc((void **)&display->gl.Viewport, "glViewport")) {
        return 0;
    }
    return 1;
}

static int uya_gui_sim_compile_gles2_shader(UyaGuiSimDisplay *display, GLenum type, const char *source, GLuint *out_shader) {
    GLint ok = 0;
    GLuint shader = display->gl.CreateShader(type);
    if (shader == 0u) {
        uya_gui_sim_set_error("glCreateShader failed");
        return 0;
    }
    display->gl.ShaderSource(shader, 1, (const GLchar *const *)&source, NULL);
    display->gl.CompileShader(shader);
    display->gl.GetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok == 0) {
        GLchar info[512];
        GLsizei info_len = 0;
        info[0] = '\0';
        display->gl.GetShaderInfoLog(shader, (GLsizei)sizeof(info), &info_len, info);
        display->gl.DeleteShader(shader);
        uya_gui_sim_set_error(info_len > 0 ? (const char *)info : "glCompileShader failed");
        return 0;
    }
    *out_shader = shader;
    return 1;
}

static int uya_gui_sim_init_gles2_pipeline(UyaGuiSimDisplay *display) {
    static const char *k_vertex_shader =
        "attribute vec2 a_pos;\n"
        "attribute vec2 a_uv;\n"
        "varying vec2 v_uv;\n"
        "void main() {\n"
        "  gl_Position = vec4(a_pos, 0.0, 1.0);\n"
        "  v_uv = a_uv;\n"
        "}\n";
    static const char *k_fragment_shader =
        "precision mediump float;\n"
        "varying vec2 v_uv;\n"
        "uniform sampler2D u_tex;\n"
        "void main() {\n"
        "  gl_FragColor = texture2D(u_tex, v_uv);\n"
        "}\n";
    static const GLfloat k_quad_vertices[] = {
        -1.0f,  1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 1.0f,
    };
    GLint ok = 0;
    size_t rgba_bytes = (size_t)display->width * (size_t)display->height * 4u;
    GLuint vertex_shader = 0u;
    GLuint fragment_shader = 0u;

    if (!uya_gui_sim_load_gles2_functions(display)) {
        return 0;
    }
    if (!uya_gui_sim_compile_gles2_shader(display, GL_VERTEX_SHADER, k_vertex_shader, &vertex_shader)) {
        return 0;
    }
    if (!uya_gui_sim_compile_gles2_shader(display, GL_FRAGMENT_SHADER, k_fragment_shader, &fragment_shader)) {
        display->gl.DeleteShader(vertex_shader);
        return 0;
    }

    display->gl_program = display->gl.CreateProgram();
    if (display->gl_program == 0u) {
        display->gl.DeleteShader(vertex_shader);
        display->gl.DeleteShader(fragment_shader);
        uya_gui_sim_set_error("glCreateProgram failed");
        return 0;
    }
    display->gl.AttachShader(display->gl_program, vertex_shader);
    display->gl.AttachShader(display->gl_program, fragment_shader);
    display->gl.LinkProgram(display->gl_program);
    display->gl.GetProgramiv(display->gl_program, GL_LINK_STATUS, &ok);
    display->gl.DeleteShader(vertex_shader);
    display->gl.DeleteShader(fragment_shader);
    if (ok == 0) {
        GLchar info[512];
        GLsizei info_len = 0;
        info[0] = '\0';
        display->gl.GetProgramInfoLog(display->gl_program, (GLsizei)sizeof(info), &info_len, info);
        uya_gui_sim_set_error(info_len > 0 ? (const char *)info : "glLinkProgram failed");
        return 0;
    }

    display->gl_attr_pos = display->gl.GetAttribLocation(display->gl_program, "a_pos");
    display->gl_attr_uv = display->gl.GetAttribLocation(display->gl_program, "a_uv");
    display->gl_uniform_tex = display->gl.GetUniformLocation(display->gl_program, "u_tex");
    if (display->gl_attr_pos < 0 || display->gl_attr_uv < 0 || display->gl_uniform_tex < 0) {
        uya_gui_sim_set_error("gles2 shader attribute lookup failed");
        return 0;
    }

    display->gl.GenTextures(1, &display->gl_texture);
    display->gl.BindTexture(GL_TEXTURE_2D, display->gl_texture);
    display->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    display->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    display->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    display->gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    display->gl.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    display->gl.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, display->width, display->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    display->gl.GenBuffers(1, &display->gl_vbo);
    display->gl.BindBuffer(GL_ARRAY_BUFFER, display->gl_vbo);
    display->gl.BufferData(GL_ARRAY_BUFFER, (GLsizeiptr)sizeof(k_quad_vertices), k_quad_vertices, GL_STATIC_DRAW);
    display->gl.BindBuffer(GL_ARRAY_BUFFER, 0u);

    display->gl_rgba_pixels = (uint8_t *)uya_gui_sim_host_malloc(rgba_bytes);
    if (display->gl_rgba_pixels == NULL) {
        uya_gui_sim_set_error("malloc gles2 scratch failed");
        return 0;
    }
    display->gl_rgba_bytes = rgba_bytes;

    display->gl.ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    display->present_kind = UYA_GUI_SIM_PRESENT_GLES2;
    return 1;
}

static int uya_gui_sim_init_renderer_pipeline(UyaGuiSimDisplay *display) {
    (void)SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    display->renderer = SDL_CreateRenderer(display->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (display->renderer == NULL) {
        display->renderer = SDL_CreateRenderer(display->window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (display->renderer == NULL) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }
    SDL_RenderSetLogicalSize(display->renderer, display->width, display->height);
    display->texture = SDL_CreateTexture(
        display->renderer,
        SDL_PIXELFORMAT_BGRA8888,
        SDL_TEXTUREACCESS_STREAMING,
        display->width,
        display->height
    );
    if (display->texture == NULL) {
        uya_gui_sim_set_error(SDL_GetError());
        uya_gui_sim_destroy_renderer_resources(display);
        return 0;
    }
#if SDL_VERSION_ATLEAST(2, 0, 12)
    (void)SDL_SetTextureScaleMode(display->texture, SDL_ScaleModeNearest);
#endif
    display->present_kind = UYA_GUI_SIM_PRESENT_SOFTWARE;
    return 1;
}

static int uya_gui_sim_present_gles2(UyaGuiSimDisplay *display, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height) {
    int drawable_w = 0;
    int drawable_h = 0;

    if (display->gl_context == NULL || display->window == NULL || display->gl_rgba_pixels == NULL) {
        uya_gui_sim_set_error("gles2 backend not initialized");
        return 0;
    }
    if ((size_t)width * (size_t)height * 4u > display->gl_rgba_bytes) {
        uya_gui_sim_set_error("gles2 scratch buffer too small");
        return 0;
    }

    uya_gui_sim_convert_argb_to_rgba(display->gl_rgba_pixels, pixels, pitch, width, height);
    if (SDL_GL_MakeCurrent(display->window, display->gl_context) != 0) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }
    SDL_GL_GetDrawableSize(display->window, &drawable_w, &drawable_h);
    if (drawable_w <= 0 || drawable_h <= 0) {
        drawable_w = display->width * display->scale;
        drawable_h = display->height * display->scale;
    }

    display->gl.Viewport(0, 0, drawable_w, drawable_h);
    display->gl.Clear(GL_COLOR_BUFFER_BIT);
    display->gl.UseProgram(display->gl_program);
    display->gl.ActiveTexture(GL_TEXTURE0);
    display->gl.BindTexture(GL_TEXTURE_2D, display->gl_texture);
    display->gl.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    display->gl.TexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, display->gl_rgba_pixels);
    display->gl.Uniform1i(display->gl_uniform_tex, 0);
    display->gl.BindBuffer(GL_ARRAY_BUFFER, display->gl_vbo);
    display->gl.EnableVertexAttribArray((GLuint)display->gl_attr_pos);
    display->gl.EnableVertexAttribArray((GLuint)display->gl_attr_uv);
    display->gl.VertexAttribPointer((GLuint)display->gl_attr_pos, 2, GL_FLOAT, GL_FALSE, (GLsizei)(sizeof(GLfloat) * 4), (const void *)0);
    display->gl.VertexAttribPointer((GLuint)display->gl_attr_uv, 2, GL_FLOAT, GL_FALSE, (GLsizei)(sizeof(GLfloat) * 4), (const void *)(sizeof(GLfloat) * 2));
    display->gl.DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    display->gl.DisableVertexAttribArray((GLuint)display->gl_attr_pos);
    display->gl.DisableVertexAttribArray((GLuint)display->gl_attr_uv);
    display->gl.BindBuffer(GL_ARRAY_BUFFER, 0u);
    SDL_GL_SwapWindow(display->window);
    display->needs_refresh = 0;
    return 1;
}

int32_t uya_gui_sim_sdl_display_present_begin(void *handle, int32_t width, int32_t height, int32_t format_tag) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    if (display == NULL) {
        uya_gui_sim_set_error("display null");
        return 0;
    }
    if (format_tag != 2) {
        uya_gui_sim_set_error("unsupported framebuffer format (expected ARGB8888/BGRA texture)");
        return 0;
    }
    display->pending_pixels = NULL;
    display->pending_pitch = 0;
    display->pending_width = width;
    display->pending_height = height;
    display->pending_format_tag = format_tag;
    display->pending_present = 0;
    display->dirty_overlay_full = 0;
    display->dirty_overlay_rect_count = 0;
    return 1;
}

int32_t uya_gui_sim_sdl_display_present_region(void *handle, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height, int32_t format_tag,
                                               int32_t x, int32_t y, int32_t w, int32_t h) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    SDL_Rect rect;
    const uint8_t *region_pixels;
    if (display == NULL || pixels == NULL) {
        uya_gui_sim_set_error("display/pixels null");
        return 0;
    }
    if (format_tag != 2) {
        uya_gui_sim_set_error("unsupported framebuffer format (expected ARGB8888/BGRA texture)");
        return 0;
    }
    if (w <= 0 || h <= 0) {
        return 1;
    }
    if (x < 0 || y < 0 || x + w > width || y + h > height) {
        uya_gui_sim_set_error("dirty rect out of bounds");
        return 0;
    }

    display->pending_pixels = pixels;
    display->pending_pitch = pitch;
    display->pending_width = width;
    display->pending_height = height;
    display->pending_format_tag = format_tag;

    if (display->present_kind == UYA_GUI_SIM_PRESENT_GLES2) {
        display->pending_present = 1;
        if (display->dirty_overlay_enabled) {
            if (display->dirty_overlay_rect_count < UYA_GUI_SIM_DIRTY_OVERLAY_MAX_RECTS) {
                display->dirty_overlay_rects[display->dirty_overlay_rect_count++] = (SDL_Rect){ x, y, w, h };
            } else {
                display->dirty_overlay_full = 1;
            }
        }
        return 1;
    }

    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    region_pixels = pixels + (size_t)y * (size_t)pitch + (size_t)x * 4u;
    if (SDL_UpdateTexture(display->texture, &rect, region_pixels, pitch) != 0) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }
    display->pending_present = 1;
    if (display->dirty_overlay_enabled) {
        if (display->dirty_overlay_rect_count < UYA_GUI_SIM_DIRTY_OVERLAY_MAX_RECTS) {
            display->dirty_overlay_rects[display->dirty_overlay_rect_count++] = rect;
        } else {
            display->dirty_overlay_full = 1;
        }
    }
    return 1;
}

int32_t uya_gui_sim_sdl_display_present_end(void *handle) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    const uint8_t *pixels = NULL;
    int32_t pitch = 0;
    if (display == NULL) {
        uya_gui_sim_set_error("display null");
        return 0;
    }
    if (!display->pending_present) {
        return 1;
    }
    if (display->present_kind == UYA_GUI_SIM_PRESENT_GLES2) {
        pixels = uya_gui_sim_overlay_prepare(
            display,
            display->pending_pixels,
            display->pending_pitch,
            display->pending_width,
            display->pending_height,
            &pitch
        );
        return uya_gui_sim_present_gles2(
            display,
            pixels,
            pitch,
            display->pending_width,
            display->pending_height
        );
    }
    if (!display->dirty_overlay_enabled) {
        if (SDL_RenderClear(display->renderer) != 0) {
            uya_gui_sim_set_error(SDL_GetError());
            return 0;
        }
        if (SDL_RenderCopy(display->renderer, display->texture, NULL, NULL) != 0) {
            uya_gui_sim_set_error(SDL_GetError());
            return 0;
        }
        SDL_RenderPresent(display->renderer);
        return 1;
    }
    pixels = uya_gui_sim_overlay_prepare(
        display,
        display->pending_pixels,
        display->pending_pitch,
        display->pending_width,
        display->pending_height,
        &pitch
    );
    return uya_gui_sim_present_software(display, pixels, pitch);
}

static int uya_gui_sim_keycode(SDL_Keycode sym) {
    if (sym >= 32 && sym < 127) {
        return (int)sym;
    }
    switch (sym) {
        case SDLK_ESCAPE: return 27;
        case SDLK_RETURN: return 13;
        case SDLK_SPACE: return 32;
        case SDLK_LEFT: return 1000;
        case SDLK_RIGHT: return 1001;
        case SDLK_UP: return 1002;
        case SDLK_DOWN: return 1003;
        case SDLK_F11: return 1011;
        default:
            if (sym >= 0 && sym <= 0xffff) {
                return (int)sym;
            }
            return 0;
    }
}

static void uya_gui_sim_clamp_logical_point(const UyaGuiSimDisplay *display, int in_x, int in_y, int16_t *out_x, int16_t *out_y) {
    int logical_x = in_x;
    int logical_y = in_y;
    if (display->present_kind == UYA_GUI_SIM_PRESENT_GLES2 && display->window != NULL) {
        int window_w = 0;
        int window_h = 0;
        SDL_GetWindowSize(display->window, &window_w, &window_h);
        if (window_w > 0 && window_h > 0) {
            logical_x = (in_x * display->width) / window_w;
            logical_y = (in_y * display->height) / window_h;
        }
    }
    if (logical_x < 0) {
        logical_x = 0;
    }
    if (logical_y < 0) {
        logical_y = 0;
    }
    if (logical_x >= display->width) {
        logical_x = display->width - 1;
    }
    if (logical_y >= display->height) {
        logical_y = display->height - 1;
    }
    *out_x = (int16_t)logical_x;
    *out_y = (int16_t)logical_y;
}

void *uya_gui_sim_sdl_display_open(int32_t width, int32_t height, int32_t scale, int32_t fullscreen, int32_t gpu_mode, const uint8_t *title) {
    uya_gui_sim_init_host_allocators();
    if (g_sdl_refcount == 0) {
        if (g_host_malloc_fn == NULL || g_host_calloc_fn == NULL || g_host_realloc_fn == NULL || g_host_free_fn == NULL) {
            uya_gui_sim_set_error("failed to resolve host allocator symbols");
            return NULL;
        }
        if (SDL_SetMemoryFunctions(
            uya_gui_sim_host_malloc,
            uya_gui_sim_host_calloc,
            uya_gui_sim_host_realloc,
            uya_gui_sim_host_free
        ) != 0) {
            uya_gui_sim_set_error(SDL_GetError());
            return NULL;
        }
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
            uya_gui_sim_set_error(SDL_GetError());
            return NULL;
        }
    }
    g_sdl_refcount += 1;

    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)uya_gui_sim_host_calloc(1, sizeof(UyaGuiSimDisplay));
    if (display == NULL) {
        uya_gui_sim_set_error("calloc display failed");
        g_sdl_refcount -= 1;
        if (g_sdl_refcount == 0) {
            SDL_Quit();
        }
        return NULL;
    }

    display->width = width;
    display->height = height;
    display->scale = scale > 0 ? scale : 1;
    display->fullscreen = fullscreen != 0;
    display->gl_attr_pos = -1;
    display->gl_attr_uv = -1;
    display->gl_uniform_tex = -1;
    display->present_kind = UYA_GUI_SIM_PRESENT_SOFTWARE;
    display->dirty_overlay_enabled = 0;
    display->dirty_overlay_full = 0;
    display->dirty_overlay_rect_count = 0;

    if (gpu_mode != UYA_GUI_SIM_GPU_SOFTWARE) {
        SDL_GL_ResetAttributes();
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        display->window = uya_gui_sim_create_window(width, height, display->scale, fullscreen, title, SDL_WINDOW_OPENGL);
        if (display->window != NULL) {
            display->gl_context = SDL_GL_CreateContext(display->window);
            if (display->gl_context != NULL) {
                (void)SDL_GL_SetSwapInterval(1);
                if (uya_gui_sim_init_gles2_pipeline(display)) {
                    g_active_display = display;
                    g_last_error[0] = '\0';
                    return display;
                }
            } else {
                uya_gui_sim_set_error(SDL_GetError());
            }
        } else {
            uya_gui_sim_set_error(SDL_GetError());
        }
        if (gpu_mode == UYA_GUI_SIM_GPU_GLES2) {
            uya_gui_sim_destroy_display_resources(display);
            uya_gui_sim_host_free(display);
            g_sdl_refcount -= 1;
            if (g_sdl_refcount == 0) {
                SDL_Quit();
            }
            return NULL;
        }
        uya_gui_sim_destroy_display_resources(display);
    }

    display->window = uya_gui_sim_create_window(width, height, display->scale, fullscreen, title, 0u);
    if (display->window == NULL || !uya_gui_sim_init_renderer_pipeline(display)) {
        if (display->window == NULL) {
            uya_gui_sim_set_error(SDL_GetError());
        }
        uya_gui_sim_destroy_display_resources(display);
        uya_gui_sim_host_free(display);
        g_sdl_refcount -= 1;
        if (g_sdl_refcount == 0) {
            SDL_Quit();
        }
        return NULL;
    }

    g_active_display = display;
    g_last_error[0] = '\0';
    return display;
}

void uya_gui_sim_sdl_display_close(void *handle) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    if (display == NULL) {
        return;
    }
    if (g_active_display == display) {
        g_active_display = NULL;
    }
    uya_gui_sim_destroy_display_resources(display);
    uya_gui_sim_host_free(display);
    if (g_sdl_refcount > 0) {
        g_sdl_refcount -= 1;
    }
    if (g_sdl_refcount == 0) {
        SDL_Quit();
    }
}

int32_t uya_gui_sim_sdl_display_present(void *handle, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height, int32_t format_tag) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    const uint8_t *present_pixels = pixels;
    int32_t present_pitch = pitch;
    if (display == NULL || pixels == NULL) {
        uya_gui_sim_set_error("display/pixels null");
        return 0;
    }
    if (format_tag != 2) {
        uya_gui_sim_set_error("unsupported framebuffer format (expected ARGB8888/BGRA texture)");
        return 0;
    }
    display->dirty_overlay_full = display->dirty_overlay_enabled ? 1 : 0;
    display->dirty_overlay_rect_count = 0;
    present_pixels = uya_gui_sim_overlay_prepare(display, pixels, pitch, width, height, &present_pitch);
    if (display->present_kind == UYA_GUI_SIM_PRESENT_GLES2) {
        return uya_gui_sim_present_gles2(display, present_pixels, present_pitch, width, height);
    }
    return uya_gui_sim_present_software(display, present_pixels, present_pitch);
}

int32_t uya_gui_sim_sdl_display_set_dirty_overlay(void *handle, int32_t enabled) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    if (display == NULL) {
        uya_gui_sim_set_error("display null");
        return 0;
    }
    display->dirty_overlay_enabled = enabled != 0;
    display->dirty_overlay_full = 0;
    display->dirty_overlay_rect_count = 0;
    return 1;
}

int32_t uya_gui_sim_sdl_display_consume_refresh_request(void *handle) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    if (display == NULL) {
        uya_gui_sim_set_error("display null");
        return 0;
    }
    if (!display->needs_refresh) {
        return 0;
    }
    display->needs_refresh = 0;
    return 1;
}

int32_t uya_gui_sim_sdl_display_set_fullscreen(void *handle, int32_t enabled) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    if (display == NULL) {
        return 0;
    }
    uint32_t flags = enabled ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0u;
    if (SDL_SetWindowFullscreen(display->window, flags) != 0) {
        uya_gui_sim_set_error(SDL_GetError());
        return 0;
    }
    display->fullscreen = enabled != 0;
    return 1;
}

void uya_gui_sim_sdl_display_set_title(void *handle, const uint8_t *title) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    if (display == NULL || title == NULL) {
        return;
    }
    SDL_SetWindowTitle(display->window, (const char *)title);
}

const uint8_t *uya_gui_sim_sdl_display_gpu_name(void *handle) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    if (display == NULL) {
        return (const uint8_t *)"unknown";
    }
    return (const uint8_t *)uya_gui_sim_present_name(display);
}

int32_t uya_gui_sim_sdl_poll_event(SdlHostEvent *out_evt) {
    SDL_Event evt;
    if (out_evt == NULL) {
        return 0;
    }
    out_evt->kind = SDL_EVT_NONE;
    out_evt->x = 0;
    out_evt->y = 0;
    out_evt->value = 0;
    out_evt->key_code = 0;
    out_evt->reserved = 0;

    if (g_has_pending_event != 0) {
        evt = g_pending_event;
        g_has_pending_event = 0;
    } else if (SDL_PollEvent(&evt) == 0) {
        return 0;
    }
    if (evt.type == SDL_QUIT) {
        out_evt->kind = SDL_EVT_QUIT;
        return 1;
    }
    if (g_active_display == NULL) {
        return 0;
    }
    if (evt.type == SDL_WINDOWEVENT) {
        if (evt.window.event == SDL_WINDOWEVENT_EXPOSED
            || evt.window.event == SDL_WINDOWEVENT_RESTORED
            || evt.window.event == SDL_WINDOWEVENT_SHOWN
            || evt.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            g_active_display->needs_refresh = 1;
        }
        return 0;
    }

    switch (evt.type) {
        case SDL_MOUSEMOTION:
            out_evt->kind = SDL_EVT_POINTER_MOVE;
            uya_gui_sim_clamp_logical_point(g_active_display, evt.motion.x, evt.motion.y, &out_evt->x, &out_evt->y);
            return 1;
        case SDL_MOUSEBUTTONDOWN:
            if (evt.button.button == SDL_BUTTON_LEFT) {
                out_evt->kind = SDL_EVT_POINTER_DOWN;
                uya_gui_sim_clamp_logical_point(g_active_display, evt.button.x, evt.button.y, &out_evt->x, &out_evt->y);
                return 1;
            }
            return 0;
        case SDL_MOUSEBUTTONUP:
            if (evt.button.button == SDL_BUTTON_LEFT) {
                out_evt->kind = SDL_EVT_POINTER_UP;
                uya_gui_sim_clamp_logical_point(g_active_display, evt.button.x, evt.button.y, &out_evt->x, &out_evt->y);
                return 1;
            }
            return 0;
        case SDL_MOUSEWHEEL:
            out_evt->kind = SDL_EVT_WHEEL;
            out_evt->value = evt.wheel.y;
            return 1;
        case SDL_KEYDOWN:
            if (evt.key.repeat != 0) {
                return 0;
            }
            out_evt->kind = SDL_EVT_KEY_DOWN;
            out_evt->key_code = (uint16_t)uya_gui_sim_keycode(evt.key.keysym.sym);
            return 1;
        case SDL_KEYUP:
            out_evt->kind = SDL_EVT_KEY_UP;
            out_evt->key_code = (uint16_t)uya_gui_sim_keycode(evt.key.keysym.sym);
            return 1;
        default:
            return 0;
    }
}

int32_t uya_gui_sim_sdl_wait_event(int32_t timeout_ms) {
    SDL_Event evt;
    if (g_has_pending_event != 0) {
        return 1;
    }
    if (timeout_ms < 0) {
        if (SDL_WaitEvent(&evt) == 0) {
            return 0;
        }
    } else if (SDL_WaitEventTimeout(&evt, timeout_ms) == 0) {
        return 0;
    }
    g_pending_event = evt;
    g_has_pending_event = 1;
    return 1;
}

const uint8_t *uya_gui_sim_sdl_last_error(void) {
    if (g_last_error[0] != '\0') {
        return (const uint8_t *)g_last_error;
    }
    return (const uint8_t *)SDL_GetError();
}
