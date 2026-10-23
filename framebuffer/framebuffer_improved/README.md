# Draw a rectangle upon Framebuffer

## Run example:

```
python3 renderer.py
```

## Make Service

### Create User

Create a system user with access into video group:

```
sudo useradd --system --no-create-home --shell /usr/sbin/nologin framebuffer
sudo usermod -aG video framebuffer
```

### Setup Code

```
sudo mkdir -p /opt/framebuffer
sudo cp renderer.py /opt/framebuffer/renderer.py
sudo chown -R root:root /opt/framebuffer
sudo chmod 755 /opt/framebuffer
sudo chmod 644 /opt/framebuffer/renderer.py
```

#### Setup service

```
sudo cp framebuffer.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now framebuffer.service
```

## Logs

```
sudo journalctl -u framebuffer.service -b
```

## Update Service

In case you want to made changes upon `renderer.py` and want to deploy new changes run:

```
sudo systemctl disable framebuffer.service
sudo cp renderer.py /opt/framebuffer/renderer.py
sudo systemctl daemon-reload
sudo systemctl enable --now framebuffer.service
```

These steps apply also in case you want to update the service from `framebuffer/framebuffer_simple` as well. 