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
	uint32_t width = finfo.line_length/4, height = vinfo.yres;
	uint32_t yoff = vinfo.yoffset, xoff = vinfo.xoffset;
	for(y = 0; y < height; y++){
		for(x = 0; x < width; x++){
			int32_t relx = x - width/2, rely = -(y - height/2);
			double angle = atan2(rely, relx); 
			uint32_t u, v; 
			// v = (int) (((angle / M_PI) + 1.0) * 255.0 / 2.0); // Done!
			// u = (int) (1024*1024.0 / sqrt(relx*relx + rely*rely)) % 255; // Done!

			v = (int) (((angle / M_PI) + 1.0) * height / 2.0); 
			u = (int) (width * 16 / sqrt(relx*relx + rely*rely)) % width; 

			uint32_t pos = (y+yoff)*width + (x+xoff);
			uint32_t col = texture[(v+yoff)*width + (u+xoff)];
			fbp[pos] = col;
		}
	}

	sleep(1);
	free(texture);
	return 0;
}



