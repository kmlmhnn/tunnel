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

uint32_t color(uint8_t r, uint8_t g, uint8_t b, struct fb_var_screeninfo *vinfo)
{
	return r << vinfo->red.offset | g << vinfo->green.offset | b << vinfo->blue.offset ;
}


int main()
{
	int fb = open("/dev/fb0", O_RDWR);

	struct fb_fix_screeninfo finfo;
	struct fb_var_screeninfo vinfo;
	ioctl(fb, FBIOGET_FSCREENINFO, &finfo);
	ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);
	vinfo.grayscale = 0;
	vinfo.bits_per_pixel = 32;
	ioctl(fb, FBIOPUT_VSCREENINFO, &vinfo);

	long screensize = vinfo.yres_virtual * finfo.line_length;
	uint32_t *fbp = mmap(NULL, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb, (off_t)0);

	uint32_t *texture = malloc(screensize);
	memcpy(texture, fbp, screensize);

	uint32_t x, y;
	for(y = 0; y < vinfo.yres; y++){
		for(x = 0; x < vinfo.xres; x++){
			uint32_t pos = (y+vinfo.yoffset) * (finfo.line_length/4) + (x+vinfo.xoffset);
			uint32_t col = texture[(y+vinfo.yoffset)*(finfo.line_length/4) + (finfo.line_length/4)-(x+vinfo.xoffset)];
			fbp[pos] = col;
		}
	}

	sleep(1);
	free(texture);
	return 0;
}



