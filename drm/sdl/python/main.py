
import sdl2
import sdl2.ext
import sys

# Request direct DRM/KMS output.
sdl2.SDL_SetHint(b"SDL_VIDEODRIVER", b"kmsdrm")

if sdl2.SDL_Init(sdl2.SDL_INIT_VIDEO) != 0:
    print(sdl2.SDL_GetError().decode(), file=sys.stderr)
    sys.exit(1)

window = sdl2.SDL_CreateWindow(
    b"PySDL2 DRM Demo",
    sdl2.SDL_WINDOWPOS_UNDEFINED,
    sdl2.SDL_WINDOWPOS_UNDEFINED,
    1280,
    720,
    sdl2.SDL_WINDOW_FULLSCREEN
)

if not window:
    print(sdl2.SDL_GetError().decode(), file=sys.stderr)
    sdl2.SDL_Quit()
    sys.exit(1)

renderer = sdl2.SDL_CreateRenderer(
    window, -1, sdl2.SDL_RENDERER_ACCELERATED
)

if not renderer:
    print(sdl2.SDL_GetError().decode(), file=sys.stderr)
    sdl2.SDL_DestroyWindow(window)
    sdl2.SDL_Quit()
    sys.exit(1)

rect = sdl2.SDL_Rect(100, 100, 400, 200)
event = sdl2.SDL_Event()

running = True
while running:
    while sdl2.SDL_PollEvent(event):
        if event.type == sdl2.SDL_QUIT:
            running = False
        elif event.type == sdl2.SDL_KEYDOWN:
            if event.key.keysym.sym == sdl2.SDLK_ESCAPE:
                running = False

    # Dark background
    sdl2.SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255)
    sdl2.SDL_RenderClear(renderer)

    # Green rectangle
    sdl2.SDL_SetRenderDrawColor(renderer, 0, 200, 100, 255)
    sdl2.SDL_RenderFillRect(renderer, rect)

    sdl2.SDL_RenderPresent(renderer)

sdl2.SDL_DestroyRenderer(renderer)
sdl2.SDL_DestroyWindow(window)
sdl2.SDL_Quit()

