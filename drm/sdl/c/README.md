# C example using SDL


## Standalone Usage

### Install dependencies

```
sudo apt-get install libsdl2-dev
```

### Compile

```
gcc main.c -o main $(sdl2-config --cflags --libs)
```

### RUN:

```
./main
```

## Service usage


### Setup Code

```
sudo mkdir -p /opt/sdl_drm
gcc main.c -o sdl_drm_demo $(sdl2-config --cflags --libs)
sudo mv sdl_drm_demo /opt/sdl_drm/
```

### Create user

```
sudo useradd --system --no-create-home --shell /usr/sbin/nologin drmusr
sudo usermod -aG video drmusr
```

### Setup Service

```
sudo cp ./drm.service /etc/systemd/system/drm.service

# Disable framebuffer service if defined
sudo systemctl disable framebuffer.service

sudo systemctl daemon-reload
sudo systemctl enable --now drm.service

```