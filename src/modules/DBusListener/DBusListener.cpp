#include "DBusListener.h"
#include <iostream>
#include <sdbus-c++/Error.h>

template <typename T> bool update_if_changed(T &member, const T &new_value) {
  if (member != new_value) {
    member = new_value;
    return true;
  }
  return false;
}

DBusListener::DBusListener() {
  m_dbus_conn = sdbus::createSessionBusConnection();

  if (!m_dbus_conn) {
    return;
  }
  std::cout << "[DBus] Connected to D-Bus as \"" << m_dbus_conn->getUniqueName()
            << "\".\n";
  m_properties_proxy =
      sdbus::createProxy(*m_dbus_conn.get(), "org.mpris.MediaPlayer2.spotify",
                         "/org/mpris/MediaPlayer2");
  m_properties_proxy->registerSignalHandler( // subscribe to Dbus Signals
      "org.freedesktop.DBus.Properties", "PropertiesChanged",
      [this](sdbus::Signal &sig) { on_properties_changed(sig); });
  m_properties_proxy->finishRegistration();

  m_name_owner_proxy = sdbus::createProxy(
      *m_dbus_conn.get(), "org.freedesktop.DBus", "/org/freedesktop/DBus");
  m_name_owner_proxy->registerSignalHandler(
      "org.freedesktop.DBus", "NameOwnerChanged",
      [this](sdbus::Signal &sig) { on_name_owner_changed(sig); });
  m_name_owner_proxy->finishRegistration();
  m_dbus_conn->enterEventLoopAsync();

  // commented bc needed to call after observer is subscribed
  // getSpotifyInfo();
}

DBusListener::~DBusListener() {
  m_properties_proxy->unregister();
  m_name_owner_proxy->unregister();
}

void DBusListener::on_properties_changed(sdbus::Signal &signal) {
  std::string string_arg;
  std::map<std::string, sdbus::Variant> properties;
  signal >> string_arg;
  signal >> properties;
  bool isChanged = false;
  for (auto &prop : properties) {
    if (prop.first == "Metadata") {
      auto meta_v = prop.second.get<std::map<std::string, sdbus::Variant>>();
      isChanged = parseMetadata(meta_v);
    } else if (prop.first == "PlaybackStatus") {
      isChanged = update_if_changed(
          m_is_playing, (prop.second.get<std::string>() == "Playing"));
    }
  }
  if (isChanged)
    notify_observers(m_title, m_artist, m_artURL, m_spotify_started,
                     m_is_playing);
}

void DBusListener::on_name_owner_changed(sdbus::Signal &signal) {
  std::string name, old_owner, new_owner;
  signal >> name >> old_owner >> new_owner;

  bool isChanged = false;
  if (name == "org.mpris.MediaPlayer2.spotify") {
    if (new_owner.empty()) {
      std::cout << "[DBus] org.mpris.MediaPlayer2.spotify has been removed"
                << std::endl;
      isChanged = update_if_changed(m_spotify_started, false);
    } else {
      std::cout << "[DBus] org.mpris.MediaPlayer2.spotify has been created"
                << std::endl;
      isChanged = update_if_changed(m_spotify_started, true);
    }
  }
  if (isChanged)
    notify_observers(m_title, m_artist, m_artURL, m_spotify_started,
                     m_is_playing);
}

void DBusListener::getSpotifyInfo() {
  // gets spotify info, writes into variables, notifies observers
  bool isChanged = false;
  try {
    auto spotifyProxy =
        sdbus::createProxy(*m_dbus_conn.get(), "org.mpris.MediaPlayer2.spotify",
                           "/org/mpris/MediaPlayer2");
    sdbus::Variant metadata_v;
    spotifyProxy->callMethod("Get")
        .onInterface("org.freedesktop.DBus.Properties")
        .withArguments("org.mpris.MediaPlayer2.Player", "Metadata")
        .storeResultsTo(metadata_v);

    isChanged =
        parseMetadata(metadata_v.get<std::map<std::string, sdbus::Variant>>());
    isChanged = update_if_changed(m_spotify_started, true);
    auto playback = spotifyProxy->getProperty("PlaybackStatus")
                        .onInterface("org.mpris.MediaPlayer2.Player");
    isChanged = update_if_changed(m_is_playing,
                                  (playback.get<std::string>() == "Playing"));
  } catch (sdbus::Error) {
    std::cout << "[DBus] Spotify not started!" << std::endl;
    isChanged = update_if_changed(m_spotify_started, false);
    isChanged = update_if_changed(m_is_playing, false);
  }
  if (isChanged)
    notify_observers(m_title, m_artist, m_artURL, m_spotify_started,
                     m_is_playing);
}

bool DBusListener::parseMetadata(std::map<std::string, sdbus::Variant> meta) {
  // writes new info to private variables, returns whether something is updated
  std::cout << "[DBus] Started parse metadata" << std::endl;
  // refer to Crescendo project if you need more examples
  // https://github.com/PolisanTheEasyNick/Crescendo

  bool isChanged = false;
  for (auto &data : meta) {
    std::string type = data.second.peekValueType();
    if (type == "s") {
      try {
        if (data.first == "xesam:title") {
          isChanged =
              update_if_changed(m_title, data.second.get<std::string>());
          std::cout << "[DBus] Got title: " << m_title << std::endl;
        } else if (data.first == "mpris:artUrl") {
          isChanged =
              update_if_changed(m_artURL, data.second.get<std::string>());
          std::cout << "[DBus] Got Art URL: " << m_artURL << std::endl;
        }
      } catch (const sdbus::Error &e) {
        std::cout << std::string(
                         "[DBus] Error while trying to fetch string: ") +
                         e.what();
        std::cout << "[DBus] Received type: \"" + type +
                         "\" while \"s\" expected.";
        break;
      }
    } else if (type == "as") {
      if (data.first == "xesam:artist") {
        std::string artist = "";
        std::vector<std::string> arr =
            data.second.get<std::vector<std::string>>();
        for (auto &entry : arr) {
          if (artist == "" && !entry.empty())
            artist = entry;
          else if (!entry.empty())
            artist += ", " + entry;
        }
        isChanged = update_if_changed(m_artist, artist);
        std::cout << "[DBus] Got artist: " << m_artist << std::endl;
      }
    }
  }
  return isChanged;
}
