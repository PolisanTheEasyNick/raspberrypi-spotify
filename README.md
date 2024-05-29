# raspberrypi-spotify [WIP]
Show current playing song from Spotify on display connected to Raspberry Pi

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

## TODO
- [x] WS server  
- [x] DBus listener  
- [ ] gamemoderun detector from DBus (for showing monitoring info instead Spotify while playing)  
- [ ] systemctl service  
- [ ] spotifyd support 
