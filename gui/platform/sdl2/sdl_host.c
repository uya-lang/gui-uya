#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct UyaGuiSimDisplay {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    int width;
    int height;
    int scale;
    int fullscreen;
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

static void uya_gui_sim_set_error(const char *message) {
    if (message == NULL || message[0] == '\0') {
        message = "unknown SDL2 error";
    }
    (void)snprintf(g_last_error, sizeof(g_last_error), "%s", message);
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

static void uya_gui_sim_scale_point(const UyaGuiSimDisplay *display, int in_x, int in_y, int16_t *out_x, int16_t *out_y) {
    int window_w = display->width * (display->scale > 0 ? display->scale : 1);
    int window_h = display->height * (display->scale > 0 ? display->scale : 1);
    SDL_GetWindowSize(display->window, &window_w, &window_h);
    if (window_w <= 0) {
        window_w = display->width;
    }
    if (window_h <= 0) {
        window_h = display->height;
    }

    int scaled_x = (in_x * display->width) / window_w;
    int scaled_y = (in_y * display->height) / window_h;
    if (scaled_x < 0) {
        scaled_x = 0;
    }
    if (scaled_y < 0) {
        scaled_y = 0;
    }
    if (scaled_x >= display->width) {
        scaled_x = display->width - 1;
    }
    if (scaled_y >= display->height) {
        scaled_y = display->height - 1;
    }
    *out_x = (int16_t)scaled_x;
    *out_y = (int16_t)scaled_y;
}

void *uya_gui_sim_sdl_display_open(int32_t width, int32_t height, int32_t scale, int32_t fullscreen, const uint8_t *title) {
    if (g_sdl_refcount == 0) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
            uya_gui_sim_set_error(SDL_GetError());
            return NULL;
        }
    }
    g_sdl_refcount += 1;

    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)calloc(1, sizeof(UyaGuiSimDisplay));
    if (display == NULL) {
        uya_gui_sim_set_error("calloc display failed");
        SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
        g_sdl_refcount = 0;
        return NULL;
    }

    display->width = width;
    display->height = height;
    display->scale = scale > 0 ? scale : 1;
    display->fullscreen = fullscreen != 0;

    uint32_t window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
    if (display->fullscreen) {
        window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    } else {
        window_flags |= SDL_WINDOW_RESIZABLE;
    }

    display->window = SDL_CreateWindow(
        title != NULL ? (const char *)title : "UyaGUI Linux Simulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width * display->scale,
        height * display->scale,
        window_flags
    );
    if (display->window == NULL) {
        uya_gui_sim_set_error(SDL_GetError());
        free(display);
        g_sdl_refcount -= 1;
        if (g_sdl_refcount == 0) {
            SDL_Quit();
        }
        return NULL;
    }

    display->renderer = SDL_CreateRenderer(display->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (display->renderer == NULL) {
        display->renderer = SDL_CreateRenderer(display->window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (display->renderer == NULL) {
        uya_gui_sim_set_error(SDL_GetError());
        SDL_DestroyWindow(display->window);
        free(display);
        g_sdl_refcount -= 1;
        if (g_sdl_refcount == 0) {
            SDL_Quit();
        }
        return NULL;
    }

    SDL_RenderSetLogicalSize(display->renderer, width, height);
    display->texture = SDL_CreateTexture(
        display->renderer,
        SDL_PIXELFORMAT_BGRA8888,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height
    );
    if (display->texture == NULL) {
        uya_gui_sim_set_error(SDL_GetError());
        SDL_DestroyRenderer(display->renderer);
        SDL_DestroyWindow(display->window);
        free(display);
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
    if (display->texture != NULL) {
        SDL_DestroyTexture(display->texture);
    }
    if (display->renderer != NULL) {
        SDL_DestroyRenderer(display->renderer);
    }
    if (display->window != NULL) {
        SDL_DestroyWindow(display->window);
    }
    free(display);
    if (g_sdl_refcount > 0) {
        g_sdl_refcount -= 1;
    }
    if (g_sdl_refcount == 0) {
        SDL_Quit();
    }
}

int32_t uya_gui_sim_sdl_display_present(void *handle, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height, int32_t format_tag) {
    UyaGuiSimDisplay *display = (UyaGuiSimDisplay *)handle;
    (void)width;
    (void)height;
    if (display == NULL || pixels == NULL) {
        uya_gui_sim_set_error("display/pixels null");
        return 0;
    }
    if (format_tag != 2) {
        uya_gui_sim_set_error("unsupported framebuffer format (expected ARGB8888/BGRA texture)");
        return 0;
    }
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

    if (SDL_PollEvent(&evt) == 0) {
        return 0;
    }
    if (evt.type == SDL_QUIT) {
        out_evt->kind = SDL_EVT_QUIT;
        return 1;
    }
    if (g_active_display == NULL) {
        return 0;
    }

    switch (evt.type) {
        case SDL_MOUSEMOTION:
            out_evt->kind = SDL_EVT_POINTER_MOVE;
            uya_gui_sim_scale_point(g_active_display, evt.motion.x, evt.motion.y, &out_evt->x, &out_evt->y);
            return 1;
        case SDL_MOUSEBUTTONDOWN:
            if (evt.button.button == SDL_BUTTON_LEFT) {
                out_evt->kind = SDL_EVT_POINTER_DOWN;
                uya_gui_sim_scale_point(g_active_display, evt.button.x, evt.button.y, &out_evt->x, &out_evt->y);
                return 1;
            }
            return 0;
        case SDL_MOUSEBUTTONUP:
            if (evt.button.button == SDL_BUTTON_LEFT) {
                out_evt->kind = SDL_EVT_POINTER_UP;
                uya_gui_sim_scale_point(g_active_display, evt.button.x, evt.button.y, &out_evt->x, &out_evt->y);
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

const uint8_t *uya_gui_sim_sdl_last_error(void) {
    if (g_last_error[0] != '\0') {
        return (const uint8_t *)g_last_error;
    }
    return (const uint8_t *)SDL_GetError();
}
