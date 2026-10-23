import mmap
import sys

def calculate_pixel_bytes(bpp, r, g, b, a=255)->bytes:
    if bpp == 32:
        # BGRA byte order
        return bytes((b, g, r, a))

    if bpp == 16:
        # RGB565
        value = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
        return bytes((value >> 8, value & 0xff))

    raise ValueError(f"Unsupported bpp: {bpp}")

def fill_rectangle(fb,bpp,width,x1, y1, x2, y2, r, g, b, a=255):

    if x1 > x2 or y1 > y2:
        return  

    color_bytes = calculate_pixel_bytes(bpp,r,g,b,a)
    bytes_per_pixel = len(color_bytes)    

    # Rectangles are being split into distinct rows that would be drawn seperately for each line.
    row = color_bytes * (x2 - x1 + 1)
    row_size = len(row)

    for y in range(y1, y2 + 1):
        pos = (y * width + x1) * bytes_per_pixel
        fb[pos:pos + row_size] = row


def blank(fb):
    fb[:] = b'\x00' * len(fb)

fbN = 'fb0'

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

frame = bytearray(w * h * (bpp // 8))

while True:

    blank(frame)

    fill_rectangle(
        frame, bpp, w,
        100, 50, 300, 200,
        255, 0, 0
    )

    fb[:] = frame

fb.close()
fbdev.close()

