#ifndef DBUSLISTENER_H
#define DBUSLISTENER_H

#include "../Subject.h"
#include <sdbus-c++/Message.h>
#include <sdbus-c++/sdbus-c++.h>

class DBusListener : public Subject {
private:
  std::unique_ptr<sdbus::IConnection> m_dbus_conn;
  std::unique_ptr<sdbus::IProxy> m_spotify_properties_proxy, m_name_owner_proxy,
      m_gamemode_proxy, m_spotifyd_properties_proxy;
  void on_spotify_prop_changed(sdbus::Signal &signal);
  void on_name_owner_changed(sdbus::Signal &signal);
  void on_game_prop_changed(sdbus::Signal &signal);
  bool parseMetadata(std::map<std::string, sdbus::Variant> meta);
  std::string findSpotifyd();

  // spotify related data
  std::string m_artURL;
  std::string m_artist;
  std::string m_title;
  std::string m_album;
  bool m_spotify_started;
  bool m_is_playing;
  bool m_is_gamemode_running = false;

public:
  DBusListener();
  ~DBusListener();
  void getSpotifyInfo();
};

#endif // DBUSLISTENER.H