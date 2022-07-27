# Linux 'Hi!' Example
This is a something small I did to see if I could make a Linux system from
scratch (although this one doesn't do much at all). The explanations below will
require you to have some background knowlege, but if you kind-of understand how
to compile a Linux kernel (just google it) and how Linux starts up with an
initramfs and runs an init process, you'll be fine.

Note: I haven't compiled a Linux kernel before this, so I'm probably doing
*something* wrong.

### Compiling Linux and Generating an initramfs on macOS
While doing this I ran into some problems: 1) you can't just compile a Linux
kernel on macOS, and 2) you can't create an initramfs on macOS because you
can't create device special files for Linux with mknod on macOS.

To solve the first problem, I compiled the Linux kernel in a Docker container
running Alpine Linux with a bunch of packages installed to make stuff work\*.
I'm not sure if I compiled it with too many things enabled, if I'm missing out
on some trick to speed up compilations, if Docker is slow, if my computer is
slow, or if the kernel is just really big, but compiling this took me about
three hours (which I ran overnight). Subsequent compilations might be faster
if you use [ccache](https://ccache.dev), but I've only compiled it once so far.

Solving the second problem was harder, although it didn't require compiling for
three hours. Even in a Docker container, I wasn't able to create the device
special files needed for an initramfs. To circumvent this, I create a virtual
machine in qemu running Linux (Alpine Linux again, but this is just my distro
of choice). I then created the initramfs in the virtual machine and uploaded
the completed cpio archive to https://0x0.st. I downloaded this initramfs on my
host machine and everything was OK.

\* I installed everything in
https://wiki.alpinelinux.org/wiki/How_to_get_regular_stuff_working along with
`linux-headers` and `diffutils`. I might be forgetting some stuff or installed
too many things, but this compiled the Linux kernel for me without any errors.

### Running
I used qemu to test if this works. I intented to use qemu all along so I
disabled anything in the kernel that I didn't think would make a difference in
qemu.
`qemu-system-x86_64 -kernel bzImage -initrd initramfs.cpio -append
"root=/dev/ram init=/init"`

### Files
- `bzImage`: compiled Linux 5.15.55 kernel image. 
- `.config`: kernel .config file used while compiling. shows which features
	have been enabled or (more likely) disabled while compiling
- `initramfs.cpio`: an initramfs file. I'm not going to explain what an
	initramfs is (google it), but this one contains an init program I stole
	from a blog (first reference below) which prints "Hi!" once per second
	indefinetly. I configured my kernel to not support a compressed initramfs,
	so this is an uncompressed cpio archive.
- `output.txt`: the output I got when I ran this in qemu. to get the text
	output in qemu I ran `qemu-system-x86_64 -kernel bzImage -initrd
	initramfs.cpio -nographic -append "root=/dev/ram init=/init
	console=ttyS0"`. Warning: If you do this, you can't simply Control-C out of
	qemu. You'll need to open a new terminal and use the `kill` command to stop
	it.

### References
While trying to make this work I used the following blog posts:
- https://ibug.io/blog/2019/04/os-lab-1/
- https://hernandigiorgi.com/how-to-create-an-initramfs-after-you-compile-a-linux-kernel/
If you try to do this, these links are helpful.

