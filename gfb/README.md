# gfb
gfb is an example of using the Linux framebuffer. can draw rectangles,
outlines of rectangles, and render text with .psf/.psfu fonts (PC Screen
Fonts). Only works on Linux.

### Dependences
- [zlib](https://zlib.net)
- Linux framebuffer
If compiling you'll also need the the zlib and linux headers. these are
probably `zlib-dev` (or possibly)`zlib-devel`) and `linux-headers` in your package manager.

### Files
- `default8x16.psfu.gz`: the default framebuffer console font. pulled from
	Alpine Linux. is gzip'd but works with this example because I used zlib.
- `sans6x12.psf`: another font that works with this program. not gzip'd but
	still works becauce zlib is cool with it.
- `gfb`: the precompiled program for x86_64. takes one option parameter: a
	path to a psf font. if a psf font is not given, it will look in
	`/usr/share/consolefonts/default8x16.psfu.gz` (this might only work on
	Alpine Linux). examples: `./gfb sans6x12.psf`, `./gfb
	default8x16.psfu.gz`, `./gfb`
- `gfb.c`: source code for `gfb`. if you want to compile this, make sure to
	install the dependences above.

### References
- https://cmcenroe.me/2018/01/30/fbclock.html

