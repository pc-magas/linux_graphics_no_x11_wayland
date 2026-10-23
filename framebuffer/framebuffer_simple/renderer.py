import mmap # shared memory
import sys  # exit

def set_pixel(fb,bpp,x, y, r, g, b, a=255):
    if x < 0 or x >= w or y < 0 or y >= h:
        return

    if bpp == 32:
        # Your framebuffer appears to use BGRA byte order
        p = bytes((b, g, r, a))
    else:
        # RGB565
        value = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
        p = bytes((value >> 8, value & 0xff))

    pos = (y * w + x) * len(p)
    fb[pos:pos + len(p)] = p


def draw_rectangle(fb,bpp,x1, y1, x2, y2, r, g, b, a=255):
    # Top and bottom
    for x in range(x1, x2 + 1):
        set_pixel(fb,bpp,x, y1, r, g, b, a)
        set_pixel(fb,bpp,x, y2, r, g, b, a)

    # Left and right
    for y in range(y1, y2 + 1):
        set_pixel(fb,bpp,x1, y, r, g, b, a)
        set_pixel(fb,bpp,x2, y, r, g, b, a)

def fill_rectangle(fb,bpp,x1, y1, x2, y2, r, g, b, a=255):
    for y in range(y1, y2 + 1):
        for x in range(x1, x2 + 1):
            set_pixel(fb,bpp,x, y, r, g, b, a)

fbN = 'fb0'   # device created by the device driver

# get width and height
f = open( f"/sys/class/graphics/{fbN}/virtual_size", "r")
wh = f.read()
wa,ha = wh.split( ',')
w = int( wa) # width
h = int( ha) # height
f.close()

# get bits per pixel
f = open( f"/sys/class/graphics/{fbN}/bits_per_pixel", "r")
bpp = int( f.read())
if not bpp in (16,32):
  print( "Unsupported bpp")
  sys.exit()
f.close()

# open framebuffer and map it onto a python bytearray
fbdev = open( f"/dev/{fbN}", mode='r+b') # open R/W
fb = mmap.mmap( fbdev.fileno(), w * h * bpp//8, mmap.MAP_SHARED, mmap.PROT_WRITE|mmap.PROT_READ)

fill_rectangle(fb,bpp,0,0,w,h,0,0,0,0)
fill_rectangle(fb, bpp, 100, 50, 300, 200, 255, 0, 0)

fb.close()
fbdev.close()

