#include "DBusListener.h"
#include <iostream>

DBusListener::DBusListener() {
  m_dbus_conn = sdbus::createSessionBusConnection();
  if (!m_dbus_conn) {
    return;
  }
  std::cout << "[DBus] Connected to D-Bus as \"" << m_dbus_conn->getUniqueName()
            << "\".\n";
  subscribe(); // to spotify

  auto proxy = sdbus::createProxy(*m_dbus_conn.get(), "org.freedesktop.DBus",
                                  "/org/freedesktop/DBus");
  proxy->registerSignalHandler(
      "org.freedesktop.DBus", "NameOwnerChanged",
      [this](sdbus::Signal &sig) { on_name_owner_changed(sig); });
  proxy->finishRegistration();
  m_dbus_conn->enterEventLoopAsync();
}

void DBusListener::on_properties_changed(sdbus::Signal &signal) {
  std::string string_arg;
  std::map<std::string, sdbus::Variant> properties;
  std::vector<std::string> array_of_strings;
  signal >> string_arg;
  signal >> properties;
  for (auto &prop : properties) {
    // refer to Crescendo project if you need more examples
    // https://github.com/PolisanTheEasyNick/Crescendo
    if (prop.first == "Metadata") {
      auto meta_v = prop.second.get<std::map<std::string, sdbus::Variant>>();

      for (auto &data : meta_v) {
        std::string type = data.second.peekValueType();
        if (type == "s") {
          try {
            if (data.first == "xesam:title") {
              m_title = data.second.get<std::string>();
              std::cout << "[DBus] New title: " << m_title << std::endl;
            } else if (data.first == "mpris:artUrl") {
              m_artURL = data.second.get<std::string>();
              std::cout << "[DBus] New Art URL: " << m_artURL << std::endl;
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
            m_artist = "";
            std::vector<std::string> arr =
                data.second.get<std::vector<std::string>>();
            for (auto &entry : arr) {
              if (m_artist == "" && !entry.empty())
                m_artist = entry;
              else if (!entry.empty())
                m_artist += ", " + entry;
            }
            std::cout << "[DBus] New artist: " << m_artist << std::endl;
          }
        }
      }
      notify_observers(m_title, m_artist, m_artURL);
    }
  }
}

void DBusListener::on_name_owner_changed(sdbus::Signal &signal) {
  std::string name, old_owner, new_owner;
  signal >> name >> old_owner >> new_owner;

  if (name == "org.mpris.MediaPlayer2.spotify") {
    // TODO: Send these updates
    if (new_owner.empty()) {
      std::cout << "[DBus] org.mpris.MediaPlayer2.spotify has been removed"
                << std::endl;
    } else {
      std::cout << "[DBus] org.mpris.MediaPlayer2.spotify has been created"
                << std::endl;
    }
  }
}

void DBusListener::subscribe() {
  m_proxy_signal =
      sdbus::createProxy(*m_dbus_conn.get(), "org.mpris.MediaPlayer2.spotify",
                         "/org/mpris/MediaPlayer2");
  m_proxy_signal->registerSignalHandler( // subscribe to Dbus Signals
      "org.freedesktop.DBus.Properties", "PropertiesChanged",
      [this](sdbus::Signal &sig) { on_properties_changed(sig); });
  m_proxy_signal->finishRegistration();
}

void DBusListener::unsubscribe() { m_proxy_signal->unregister(); }