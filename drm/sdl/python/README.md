# Python example using SDL

A simple python example using virtualenv

## Install python

```
sudo apt-get install python3 python3-venv python3-pip
```

## Install dependencies

```
pip install -r requirements.txt
```

## RUN:

### Without virtualenv

```
python3 main.py
```

### With virtualenv

```
python -m venv ./.venv
source ./.venv/bin/activate
pip install -r requirements.txt
python main.py
```

## Systemd Service:

## Deploy Code

```
mkdir /opt/sdl_drm_py
sudo cp main.py /opt/sdl_drm_py/
sudo cp requirements.txt /opt/sdl_drm_py/
cd /opt/sdl_drm_py
python -m venv ./.venv
/opt/sdl_drm_py/.venv/pip install -r requirements.txt 
```

## Create user

```
sudo useradd --system --no-create-home --shell /usr/sbin/nologin drmusr
sudo usermod -aG video drmusr
```

## Create Service:

```
sudo cp ./drmpy.service /etc/systemd/system/drmpy.service

# Disable framebuffer service if defined
sudo systemctl disable framebuffer.service

# Disable previous drm service if defined
sudo systemctl disable drm.service

sudo systemctl daemon-reload
sudo systemctl enable --now drmpy.service
```