# raspberrypi-spotify [WIP]
Show current playing song from Spotify on display connected to Raspberry Pi.  
Currently hard-coded to 480x320 display, but you can easily edit it in .css and .js file!

<p align="center">
  <img src="https://github.com/PolisanTheEasyNick/raspberrypi-spotify/assets/39007846/be85a817-5478-47c3-8195-564739d86425" width=50% height=50%>
</p>

## Dependencies
* sdbus-c++
* websocketpp

## How it works?
Using DBus it connects to local Spotify app and subscribes to updates for title, artist and album artURL.  
It opens WebSocket server on port 4831 from where you can get all this data.  
Also WebSocket provides data whether Spotify opened and whether song is currently played for future automations (auto-open web page with current song when spotify plays and so on).

## How to build and run?
```
git clone https://github.com/PolisanTheEasyNick/raspberrypi-spotify
mkdir build
cd build
cmake ..
make
./raspberrypi-spotify
```
For installing to system run:
`sudo make install`  
It will install binary to `/usr/local/bin/raspberrypi-spotify` and will create systemd service in `/etc/systemd/user/rasp-spot`.  
For starting service on system boot you can use:
`systemctl --user enable --now rasp-spot`

## TODO
- [x] WS server  
- [x] DBus listener  
- [ ] gamemoderun detector from DBus (for showing monitoring info instead Spotify while playing)  
- [x] systemctl service  
- [ ] spotifyd support 
