#ifndef DBUSLISTENER_H
#define DBUSLISTENER_H

#include "../Subject.h"
#include <sdbus-c++/Message.h>
#include <sdbus-c++/sdbus-c++.h>

class DBusListener : public Subject {
private:
  std::unique_ptr<sdbus::IConnection> m_dbus_conn;
  std::unique_ptr<sdbus::IProxy> m_properties_proxy, m_name_owner_proxy;
  void on_properties_changed(sdbus::Signal &signal);
  void on_name_owner_changed(sdbus::Signal &signal);
  bool parseMetadata(std::map<std::string, sdbus::Variant> meta);

  // spotify related data
  std::string m_artURL;
  std::string m_artist;
  std::string m_title;
  std::string m_album;
  bool m_spotify_started;
  bool m_is_playing;

public:
  DBusListener();
  ~DBusListener();
  void getSpotifyInfo();
};

#endif // DBUSLISTENER.H