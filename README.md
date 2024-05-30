# raspberrypi-spotify
Show current playing song from Spotify on display connected to Raspberry Pi.  
Currently hard-coded to 480x320 display, but you can easily edit it in .css and .js files!


<p align="center">
  <img src="https://github.com/PolisanTheEasyNick/raspberrypi-spotify/assets/39007846/c6089daf-8501-42df-b277-eb4b8873c2f5" width=50% height=50%>
</p>

## Dependencies
* [sdbus-c++](https://github.com/Kistler-Group/sdbus-cpp) (libsdbus-c++-dev for building on Raspberry Pi OS)
* [websocketpp](https://github.com/zaphoyd/websocketpp) (libwebsocketpp-dev for building on Raspberry Pi OS)

## How it works?
Using DBus it connects to local Spotify app and subscribes to updates for title, artist and album artURL.  
It opens WebSocket server on port 4831 from where you can get all this data.  
### What it returns?
`title` - current song title  
`artist` - current song artist  
`album` - current song album name  
`artURL` - current song album art URL  
`spotifyStarted` - returns "True" if spotify(d) application is started, "False" otherwise. Note that "True" or "False" passed as strings.  
`isPlaying` - whether song in spotify(d) is playing. Note if spotifyd is discovered in spotify app on PC but song plays on PC then spotifyd will return that song NOT played (bc it's not playing in spotifyd).  
`gamemodeStarted` - returns "True" if gamemode started. You can use it for not displaying spotify when playing games (and show some monitoring info instead).  

## How to build and run server?
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
It will install binary to `/usr/local/bin/raspberrypi-spotify` and will create systemd service in `/etc/systemd/user/rasp-spot.service`.  
For starting service on system boot you can use:
`systemctl --user enable --now rasp-spot`

## How to use with spotifyd?
Firstly, make sure that spotifyd builded with `dbus_mpris` feature flag.  
Then just build and start raspberrypi-spotify on your Raspberry Pi. 

## How to see what i'm listening right now?
For displaying info just open src/web/index.html in any WebBrowser.
