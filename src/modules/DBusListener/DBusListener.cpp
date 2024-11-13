#include "DBusListener.h"
#include <iostream>
#include <memory>
#include <sdbus-c++/Error.h>
#include <sdbus-c++/IProxy.h>

#ifdef PILED
std::atomic<bool> Subject::stopRequested(false);
#endif

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

  // registering to spotify updates
  sdbus::ServiceName spotifyDestination{"org.mpris.MediaPlayer2.spotify"};
  sdbus::ObjectPath spotifyObjectPath{"/org/mpris/MediaPlayer2"};
  m_spotify_properties_proxy =
      sdbus::createProxy(*m_dbus_conn, spotifyDestination, spotifyObjectPath);
  sdbus::InterfaceName PropertiesInterfaceName{
      "org.freedesktop.DBus.Properties"};
  sdbus::SignalName PropertiesChangedSignalName{"PropertiesChanged"};
  m_spotify_properties_proxy->registerSignalHandler(
      PropertiesInterfaceName, PropertiesChangedSignalName,
      [this](sdbus::Signal sig) { on_spotify_prop_changed(sig); });

  // registering to spotify closed/open
  sdbus::ServiceName dbusDestination{"org.freedesktop.DBus"};
  sdbus::ObjectPath dbusObjectPath{"/org/freedesktop/DBus"};
  m_name_owner_proxy =
      sdbus::createProxy(*m_dbus_conn, dbusDestination, dbusObjectPath);
  sdbus::InterfaceName DBusInterfaceName{"org.freedesktop.DBus"};
  sdbus::SignalName NameOwnerChangedSignalName{"NameOwnerChanged"};
  m_name_owner_proxy->registerSignalHandler(
      DBusInterfaceName, NameOwnerChangedSignalName,
      [this](sdbus::Signal sig) { on_name_owner_changed(sig); });

  // registering to gamemoderun changes
  sdbus::ServiceName gameModeDestination{"com.feralinteractive.GameMode"};
  sdbus::ObjectPath gameModeObjectPath{"/com/feralinteractive/GameMode"};
  m_gamemode_proxy =
      sdbus::createProxy(*m_dbus_conn, spotifyDestination, spotifyObjectPath);
  m_gamemode_proxy->registerSignalHandler(
      DBusInterfaceName, NameOwnerChangedSignalName,
      [this](sdbus::Signal sig) { on_game_prop_changed(sig); });

  // searching for spotifyd
  std::string spotifydDest = findSpotifyd();
  if (spotifydDest != "") {
    std::cout << "[DBus] Found spotifyd: " << spotifydDest << std::endl;
    // register to it prop changed
    sdbus::ServiceName spotifyDDestination{spotifydDest};
    m_spotifyd_properties_proxy =
        sdbus::createProxy(*m_dbus_conn, spotifyDestination, spotifyObjectPath);
    m_spotifyd_properties_proxy->registerSignalHandler(
        PropertiesInterfaceName, PropertiesChangedSignalName,
        [this](sdbus::Signal sig) { on_spotify_prop_changed(sig); });
  }

  m_dbus_conn->enterEventLoopAsync();
}

DBusListener::~DBusListener() {
  m_spotify_properties_proxy->unregister();
  m_name_owner_proxy->unregister();
  m_gamemode_proxy->unregister();
  m_spotifyd_properties_proxy->unregister();
}

void DBusListener::on_spotify_prop_changed(sdbus::Signal &signal) {
  std::string string_arg;
  std::map<std::string, sdbus::Variant> properties;
  signal >> string_arg;
  signal >> properties;
  bool isChanged = false;
  for (auto &prop : properties) {
    if (prop.first == "Metadata") {
      auto meta_v = prop.second.get<std::map<std::string, sdbus::Variant>>();
      isChanged |= parseMetadata(meta_v);
    } else if (prop.first == "PlaybackStatus") {
      isChanged |= update_if_changed(
          m_is_playing, (prop.second.get<std::string>() == "Playing"));
    }
  }
  std::cout << "[dbus] on spotify prop changed is changed: " << isChanged
            << std::endl;
  if (isChanged)
    notify_observers(m_title, m_artist, m_album, m_artURL, m_spotify_started,
                     m_is_playing, m_is_gamemode_running);
}

void DBusListener::on_name_owner_changed(sdbus::Signal &signal) {
  std::string name, old_owner, new_owner;
  signal >> name >> old_owner >> new_owner;
  bool isChanged = false;
  if (name == "org.mpris.MediaPlayer2.spotify") {
    if (new_owner.empty()) {
      std::cout << "[DBus] org.mpris.MediaPlayer2.spotify has been removed"
                << std::endl;
      isChanged |= update_if_changed(m_spotify_started, false);
      m_spotify_properties_proxy->unregister();
    } else {
      sdbus::ServiceName spotifyDestination{"org.mpris.MediaPlayer2.spotify"};
      sdbus::ObjectPath spotifyObjectPath{"/org/mpris/MediaPlayer2"};
      m_spotify_properties_proxy = sdbus::createProxy(
          *m_dbus_conn, spotifyDestination, spotifyObjectPath);
      sdbus::InterfaceName PropertiesInterfaceName{
          "org.freedesktop.DBus.Properties"};
      sdbus::SignalName PropertiesChangedSignalName{"PropertiesChanged"};
      m_spotify_properties_proxy->registerSignalHandler(
          PropertiesInterfaceName, PropertiesChangedSignalName,
          [this](sdbus::Signal sig) { on_spotify_prop_changed(sig); });
      std::cout << "[DBus] org.mpris.MediaPlayer2.spotify has been created"
                << std::endl;
      isChanged |= update_if_changed(m_spotify_started, true);
      getSpotifyInfo();
    }
  } else if (name.find("org.mpris.MediaPlayer2.spotifyd") == 0) {
    if (new_owner.empty()) {
      std::cout << "[DBus] " << name << " has been removed" << std::endl;
      isChanged |= update_if_changed(m_spotify_started, false);
      m_spotifyd_properties_proxy->unregister();
    } else {
      std::cout << "[DBus] " << name << " has been created" << std::endl;
      isChanged |= update_if_changed(m_spotify_started, true);
      sdbus::ServiceName spotifyDDestination{name};
      sdbus::InterfaceName PropertiesInterfaceName{
          "org.freedesktop.DBus.Properties"};
      sdbus::SignalName PropertiesChangedSignalName{"PropertiesChanged"};
      sdbus::ObjectPath spotifyObjectPath{"/org/mpris/MediaPlayer2"};
      m_spotifyd_properties_proxy = sdbus::createProxy(
          *m_dbus_conn, spotifyDDestination, spotifyObjectPath);
      m_spotifyd_properties_proxy->registerSignalHandler(
          PropertiesInterfaceName, PropertiesChangedSignalName,
          [this](sdbus::Signal sig) { on_spotify_prop_changed(sig); });
      getSpotifyInfo();
    }
  }
  if (isChanged)
    notify_observers(m_title, m_artist, m_album, m_artURL, m_spotify_started,
                     m_is_playing, m_is_gamemode_running);
}

void DBusListener::on_game_prop_changed(sdbus::Signal &signal) {
  std::cout << "[GameMode] Game prop changed!" << std::endl;
  std::string string_arg;
  std::map<std::string, sdbus::Variant> properties;
  signal >> string_arg;
  signal >> properties;
  std::cout << "[GameMode] Client count: "
            << properties["ClientCount"].get<int>() << std::endl;

  // update variable and notify if updated
  if (update_if_changed(m_is_gamemode_running,
                        properties["ClientCount"].get<int>() > 0))
    notify_observers(m_title, m_artist, m_album, m_artURL, m_spotify_started,
                     m_is_playing, m_is_gamemode_running);
}

void DBusListener::getSpotifyInfo() {
  // gets spotify info, writes into variables, notifies observers
  bool isChanged = false;
  std::unique_ptr<sdbus::IProxy> spotifyProxy;
  try {
    bool spotifyFound = false;
    sdbus::Variant metadata_v;
    try {
      sdbus::ServiceName spotifyDestination{"org.mpris.MediaPlayer2.spotify"};
      sdbus::ObjectPath spotifyObjectPath{"/org/mpris/MediaPlayer2"};
      spotifyProxy = sdbus::createProxy(spotifyDestination, spotifyObjectPath);
      spotifyProxy->callMethod("Get")
          .onInterface("org.freedesktop.DBus.Properties")
          .withArguments("org.mpris.MediaPlayer2.Player", "Metadata")
          .storeResultsTo(metadata_v);
      spotifyFound = true;
    } catch (sdbus::Error) {
      std::cout << "[DBus] Spotify not started! Searching for spotifyd..."
                << std::endl;
    }

    std::string spotifydDest = findSpotifyd();
    if (spotifydDest != "") {
      std::cout << "[DBus] spotifyd found!" << std::endl;
      try {
        sdbus::ServiceName spotifyDDestination{spotifydDest};
        sdbus::ObjectPath spotifyObjectPath{"/org/mpris/MediaPlayer2"};
        spotifyProxy = sdbus::createProxy(*m_dbus_conn, spotifyDDestination,
                                          spotifyObjectPath);
        spotifyProxy = sdbus::createProxy(*m_dbus_conn, spotifyDDestination,
                                          spotifyObjectPath);
        spotifyProxy->callMethod("Get")
            .onInterface("org.freedesktop.DBus.Properties")
            .withArguments("org.mpris.MediaPlayer2.Player", "Metadata")
            .storeResultsTo(metadata_v);
        spotifyFound = true;
      } catch (sdbus::Error) {
        std::cout << "[DBus] Spotify not started! Searching for spotifyd..."
                  << std::endl;
      }
    }
    m_spotify_started = spotifyFound;

    isChanged |=
        parseMetadata(metadata_v.get<std::map<std::string, sdbus::Variant>>());
    isChanged |= update_if_changed(m_spotify_started, true);
    auto playback = spotifyProxy->getProperty("PlaybackStatus")
                        .onInterface("org.mpris.MediaPlayer2.Player");
    isChanged |= update_if_changed(m_is_playing,
                                   (playback.get<std::string>() == "Playing"));
  } catch (sdbus::Error) {
    std::cout << "[DBus] spotifyd not started!" << std::endl;
    isChanged |= update_if_changed(m_spotify_started, false);
    isChanged |= update_if_changed(m_is_playing, false);
  }
  if (isChanged)
    notify_observers(m_title, m_artist, m_album, m_artURL, m_spotify_started,
                     m_is_playing, m_is_gamemode_running);
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
          isChanged |=
              update_if_changed(m_title, data.second.get<std::string>());
          std::cout << "[DBus] Got title: " << m_title << std::endl;
        } else if (data.first == "mpris:artUrl") {
          isChanged |=
              update_if_changed(m_artURL, data.second.get<std::string>());
          std::cout << "[DBus] Got Art URL: " << m_artURL << std::endl;
        } else if (data.first == "xesam:album") {
          isChanged |=
              update_if_changed(m_album, data.second.get<std::string>());
          std::cout << "[DBus] Got album: " << m_album << std::endl;
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
        isChanged |= update_if_changed(m_artist, artist);
        std::cout << "[DBus] Got artist: " << m_artist << std::endl;
      }
    }
  }
  std::cout << "[DBus] Is something changed: " << isChanged << std::endl;
  return isChanged;
}

std::string DBusListener::findSpotifyd() {
  sdbus::ServiceName dbusDestination{"org.freedesktop.DBus"};
  sdbus::ObjectPath dbusObjectPath{"/org/freedesktop/DBus"};
  auto proxy =
      sdbus::createProxy(*m_dbus_conn, dbusDestination, dbusObjectPath);
  std::vector<std::string> names;
  proxy->callMethod("ListNames")
      .onInterface("org.freedesktop.DBus")
      .storeResultsTo(names);

  for (const auto &name : names) {
    if (name.find("org.mpris.MediaPlayer2.spotifyd") != std::string::npos) {
      return name;
    }
  }
  return "";
}