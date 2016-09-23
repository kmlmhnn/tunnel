#include <stdio.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

int main() {
    int fb = open("/dev/fb0", O_RDWR);

    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    ioctl(fb, FBIOGET_FSCREENINFO, &finfo);
    ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);
    vinfo.grayscale = 0;
    vinfo.bits_per_pixel = 32;
    ioctl(fb, FBIOPUT_VSCREENINFO, &vinfo);

    long screensize = vinfo.yres_virtual * finfo.line_length;
    uint32_t *fbp = mmap(NULL, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
                         fb, (off_t)0);

    uint32_t x, y;
    uint32_t width = finfo.line_length / 4, height = vinfo.yres;
    uint32_t yoff = vinfo.yoffset, xoff = vinfo.xoffset;

    uint32_t *d = malloc(screensize);
    uint32_t *a = malloc(screensize);
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int32_t relx = x - width / 2, rely = -(y - height / 2);
            double angle = atan2(rely, relx);
            uint32_t u, v;
            v = (uint32_t)(((angle / M_PI) + 1.0) * height / 2.0);
            u = (uint32_t)(width * 16 / sqrt(relx * relx + rely * rely)) %
                width;
            uint32_t pos = (y + yoff) * width + (x + xoff);
            d[pos] = u;
            a[pos] = v;
        }
    }

    uint32_t *texture = malloc(screensize);
    memcpy(texture, fbp, screensize);
    uint32_t *backbuffer = calloc(screensize, 1);
    int t;
    for (t = 0;; t++) {
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                uint32_t pos = (y + yoff) * width + (x + xoff);
                uint32_t u = (d[pos] + t) % height;
                uint32_t v = (a[pos] + t) % height;
                uint32_t col = texture[(v + yoff) * width + (u + xoff)];
                backbuffer[pos] = col;
            }
        }
        memcpy(fbp, backbuffer, screensize);
        usleep(16);
    }

    return 0;
}

