#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/fb.h>
#include <zlib.h>

#define PSF2_MAGIC 0x864AB572
struct psf2_header {
	uint32_t magic;
	uint32_t version;
	uint32_t header_size;
	uint32_t flags;
	uint32_t glyph_count;
	uint32_t glyph_size;
	uint32_t glyph_height;
	uint32_t glyph_width;
};

// same as a uint32_t
struct __attribute__((packed)) pixel {
	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t metadata;
};

int fd;
struct fb_var_screeninfo info;
size_t len;
struct pixel *buf = NULL;
struct psf2_header hdr;
uint8_t *glyphs = NULL;

static uint32_t pix2int(struct pixel pix) {
	return (pix.metadata << 24) | (pix.r << 16) | (pix.g << 8) |
		(pix.b);
}

static struct pixel int2pix(uint32_t pix) {
	struct pixel res = {.b = (uint8_t)pix, .g = (uint8_t)(pix >> 8), .r =
		(uint8_t)(pix >> 16), .metadata = (uint8_t)(pix >> 24)};
	return res;
}

void fb_init(const char *font_path) {
	fd = open("/dev/fb0", O_RDWR);
	assert(fd > 0);
	assert(ioctl(fd, FBIOGET_VSCREENINFO, &info) == 0);
	len = sizeof(struct pixel) * info.xres * info.yres;
	buf = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	assert(buf != MAP_FAILED);

	gzFile font = gzopen(font_path, "r");
	assert(font);
	assert(gzfread(&hdr, sizeof(hdr), 1, font) == 1);
	assert(hdr.magic == PSF2_MAGIC);
	glyphs = malloc(hdr.glyph_count * hdr.glyph_size);
	assert(gzfread(glyphs, hdr.glyph_size, hdr.glyph_count, font) ==
			hdr.glyph_count);
	gzclose(font);
}

void fb_end() {
	free(glyphs);
	munmap(buf, len);
	close(fd);
}

void fb_set_pixel(int x, int y, struct pixel pix) {
	buf[y*info.xres + x] = pix;
}

void fb_fill_rect(struct fb_fillrect *rect) {
	if (rect->height+rect->dy > info.yres || rect->width+rect->dx >
			info.xres)
		return;

	for (int i = rect->dy; i < rect->dy+rect->height; i++) {
		for (int j = rect->dx; j < rect->dx+rect->width; j++) {
			fb_set_pixel(j, i, int2pix(rect->color));
		}
	}
}

// uses pix.metadata as line width
void fb_rect(struct fb_fillrect *rect) {
	struct fb_fillrect top = {.dx = rect->dx, .dy = rect->dy, .width =
		rect->width, .height = int2pix(rect->color).metadata, .color =
		rect->color};
	struct fb_fillrect left = {.dx = rect->dx, .dy = rect->dy, .width =
		int2pix(rect->color).metadata, .height = rect->height, .color =
		rect->color};
	struct fb_fillrect bottom = {.dx = rect->dx, .dy = rect->dy + rect->height -
		int2pix(rect->color).metadata, .width = rect->width, .height =
		int2pix(rect->color).metadata, .color = rect->color};
	struct fb_fillrect right = {.dx = rect->dx + rect->width -
		int2pix(rect->color).metadata, .dy = rect->dy, .width =
		int2pix(rect->color).metadata, .height = rect->height, .color =
		rect->color};
	fb_fill_rect(&top);
	fb_fill_rect(&left);
	fb_fill_rect(&bottom);
	fb_fill_rect(&right);
}

void fb_char(int x, int y, char c, struct pixel pix) {
	uint8_t *glyph = glyphs + c*hdr.glyph_size;
	uint32_t stride = hdr.glyph_size / hdr.glyph_height;
	for (int i = 0; i < hdr.glyph_height; i++) {
		for (int j = 0; j < hdr.glyph_width; j++) {
			uint8_t bits = glyph[i * stride + j / 8];
			uint8_t bit = bits >> (7 - j % 8) & 1;
			if (bit)
				fb_set_pixel(x+j, y+i, pix);
		}
	}
}

void fb_print(int x, int y, const char *str, struct pixel pix) {
	while (*str) {
		fb_char(x, y, *str, pix);
		x += hdr.glyph_width;
		str++;
	}
}

int main(int argc, char *argv[]) {
	const char *font_path = "/usr/share/consolefonts/default8x16.psfu.gz";
	if (argc > 1)
		font_path = argv[1];
	fb_init(font_path);

	struct pixel green = {.g = 0xFF, .metadata = 5};
	struct pixel blue = {.b = 0xFF};
	struct pixel red = {.r = 0xFF};

	struct fb_fillrect rect = {.dx = 200, .dy = 200, .width = 100,
		.height = 100, .color = pix2int(blue)};
	fb_fill_rect(&rect);
	rect.color = pix2int(green);
	fb_rect(&rect);
	int xcoor = rect.dx + rect.width/2 - (hdr.glyph_width*5)/2;
	int ycoor = rect.dy + rect.height/2 - (hdr.glyph_height*1)/2;
	fb_print(xcoor, ycoor, "Aloha", red);
	
	fb_end();
	return 0;
}

