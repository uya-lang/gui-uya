#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct UyaGuiSimFbDisplay {
    int fd;
    uint8_t *mapped;
    size_t mapped_len;
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    char device_path[256];
    int opened;
} UyaGuiSimFbDisplay;

static UyaGuiSimFbDisplay g_fb_display = {
    .fd = -1,
    .mapped = NULL,
    .mapped_len = 0u,
    .opened = 0,
};

static char g_fb_last_error[256] = {0};

static void uya_gui_sim_fb_set_error(const char *message) {
    if (message == NULL || message[0] == '\0') {
        message = "unknown framebuffer error";
    }
    (void)snprintf(g_fb_last_error, sizeof(g_fb_last_error), "%s", message);
}

static uint32_t uya_gui_sim_scale_component(uint8_t value, uint32_t length) {
    if (length == 0u) {
        return 0u;
    }
    if (length >= 8u) {
        return ((uint32_t)value) << (length - 8u);
    }
    return (uint32_t)(value >> (8u - length));
}

static void uya_gui_sim_write_pixel(const UyaGuiSimFbDisplay *display, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    const uint32_t bits_per_pixel = display->vinfo.bits_per_pixel;
    const uint32_t bytes_per_pixel = bits_per_pixel / 8u;
    if (bytes_per_pixel == 0u) {
        return;
    }

    const size_t offset = (size_t)y * (size_t)display->finfo.line_length + (size_t)x * bytes_per_pixel;
    if (offset + bytes_per_pixel > display->mapped_len) {
        return;
    }

    uint32_t pixel = 0u;
    pixel |= uya_gui_sim_scale_component(r, display->vinfo.red.length) << display->vinfo.red.offset;
    pixel |= uya_gui_sim_scale_component(g, display->vinfo.green.length) << display->vinfo.green.offset;
    pixel |= uya_gui_sim_scale_component(b, display->vinfo.blue.length) << display->vinfo.blue.offset;
    if (display->vinfo.transp.length > 0u) {
        pixel |= uya_gui_sim_scale_component(a, display->vinfo.transp.length) << display->vinfo.transp.offset;
    }

    for (uint32_t i = 0; i < bytes_per_pixel; ++i) {
        display->mapped[offset + i] = (uint8_t)((pixel >> (i * 8u)) & 0xffu);
    }
}

void *uya_gui_sim_fb_open(const uint8_t *path) {
    UyaGuiSimFbDisplay *display = &g_fb_display;
    if (display->opened) {
        return display;
    }

    const char *env_path = getenv("UYA_GUI_FB_DEV");
    const char *dev_path = "/dev/fb0";
    if (path != NULL && path[0] != '\0') {
        dev_path = (const char *)path;
    } else if (env_path != NULL && env_path[0] != '\0') {
        dev_path = env_path;
    }

    memset(display, 0, sizeof(*display));
    display->fd = -1;
    (void)snprintf(display->device_path, sizeof(display->device_path), "%s", dev_path);

    display->fd = open(dev_path, O_RDWR);
    if (display->fd < 0) {
        uya_gui_sim_fb_set_error(strerror(errno));
        return NULL;
    }
    if (ioctl(display->fd, FBIOGET_FSCREENINFO, &display->finfo) != 0) {
        uya_gui_sim_fb_set_error(strerror(errno));
        close(display->fd);
        display->fd = -1;
        return NULL;
    }
    if (ioctl(display->fd, FBIOGET_VSCREENINFO, &display->vinfo) != 0) {
        uya_gui_sim_fb_set_error(strerror(errno));
        close(display->fd);
        display->fd = -1;
        return NULL;
    }

    display->mapped_len = (size_t)display->finfo.smem_len;
    display->mapped = mmap(NULL, display->mapped_len, PROT_READ | PROT_WRITE, MAP_SHARED, display->fd, 0);
    if (display->mapped == MAP_FAILED) {
        display->mapped = NULL;
        uya_gui_sim_fb_set_error(strerror(errno));
        close(display->fd);
        display->fd = -1;
        return NULL;
    }

    display->opened = 1;
    g_fb_last_error[0] = '\0';
    return display;
}

void uya_gui_sim_fb_close(void *handle) {
    UyaGuiSimFbDisplay *display = (UyaGuiSimFbDisplay *)handle;
    if (display == NULL || !display->opened) {
        return;
    }
    if (display->mapped != NULL) {
        (void)munmap(display->mapped, display->mapped_len);
    }
    if (display->fd >= 0) {
        (void)close(display->fd);
    }
    memset(display, 0, sizeof(*display));
    display->fd = -1;
}

int32_t uya_gui_sim_fb_present(void *handle, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height, int32_t format_tag) {
    UyaGuiSimFbDisplay *display = (UyaGuiSimFbDisplay *)handle;
    if (display == NULL || !display->opened || pixels == NULL) {
        uya_gui_sim_fb_set_error("framebuffer display not initialized");
        return 0;
    }
    if (format_tag != 2) {
        uya_gui_sim_fb_set_error("framebuffer backend only supports ARGB8888 source");
        return 0;
    }

    const int copy_w = width < (int)display->vinfo.xres ? width : (int)display->vinfo.xres;
    const int copy_h = height < (int)display->vinfo.yres ? height : (int)display->vinfo.yres;
    for (int y = 0; y < copy_h; ++y) {
        const uint8_t *row = pixels + (size_t)y * (size_t)pitch;
        for (int x = 0; x < copy_w; ++x) {
            const uint8_t *src = row + (size_t)x * 4u;
            uya_gui_sim_write_pixel(display, x, y, src[1], src[2], src[3], src[0]);
        }
    }
    return 1;
}

int32_t uya_gui_sim_fb_get_width(void *handle) {
    const UyaGuiSimFbDisplay *display = (const UyaGuiSimFbDisplay *)handle;
    if (display == NULL || !display->opened) {
        return 0;
    }
    return (int32_t)display->vinfo.xres;
}

int32_t uya_gui_sim_fb_get_height(void *handle) {
    const UyaGuiSimFbDisplay *display = (const UyaGuiSimFbDisplay *)handle;
    if (display == NULL || !display->opened) {
        return 0;
    }
    return (int32_t)display->vinfo.yres;
}

const uint8_t *uya_gui_sim_fb_last_error(void) {
    return (const uint8_t *)g_fb_last_error;
}

