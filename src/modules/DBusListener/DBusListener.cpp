#include "DBusListener.h"
#include <cstring>
#include <iostream>

DBusListener::DBusListener() {
  m_dbus_conn = sdbus::createSessionBusConnection();
  if (!m_dbus_conn) {
    return;
  }
  std::cout << "Connected to D-Bus as \"" << m_dbus_conn->getUniqueName()
            << "\".\n";
  subscribe(); // to spotify

  auto proxy = sdbus::createProxy(*m_dbus_conn.get(), "org.freedesktop.DBus",
                                  "/org/freedesktop/DBus");
  proxy->registerSignalHandler(
      "org.freedesktop.DBus", "NameOwnerChanged",
      [this](sdbus::Signal &sig) { on_name_owner_changed(sig); });
  proxy->finishRegistration();
  m_dbus_conn->enterEventLoop(); //..LoopAsync
}

void DBusListener::on_properties_changed(sdbus::Signal &signal) {
  std::map<std::string, int> property_map = {
      {"Shuffle", 1},        // Shuffle
      {"Metadata", 2},       // Changed song
      {"Volume", 3},         // changed Volume
      {"PlaybackStatus", 4}, // paused or played
      {"LoopStatus", 5}};    // Loop button status

  // Handle the PropertiesChanged signal
  std::cout << "Prop changed" << std::endl;
  std::string string_arg;
  std::map<std::string, sdbus::Variant> properties;
  std::vector<std::string> array_of_strings;
  signal >> string_arg;
  signal >> properties;
  for (auto &prop : properties) { // start parsing properties
    std::cout << prop.first << std::endl;
    switch (property_map[prop.first]) {
      // refer to Crescendo project if you need more examples
      // https://github.com/PolisanTheEasyNick/Crescendo
    case 0: { // not mapped
      std::cout << "Property \"" << prop.first << "\" not supported.";
      return;
    }
    case 2: { // metadata
      std::cout << "Metadata property changed." << std::endl;
      auto meta_v = prop.second.get<std::map<std::string, sdbus::Variant>>();
      std::map<std::string, int> type_map = {{"n", 1},    // int16
                                             {"q", 2},    // uint16
                                             {"i", 3},    // int32
                                             {"u", 4},    // uint32
                                             {"x", 5},    // int64
                                             {"t", 6},    // uint64
                                             {"d", 7},    // double
                                             {"s", 8},    // string
                                             {"o", 9},    // object path
                                             {"b", 10},   // boolean
                                             {"as", 11}}; // array of strings
      std::vector<std::pair<std::string, std::string>> metadata;

      for (auto &data : meta_v) {
        std::string type = data.second.peekValueType();
        switch (type_map[type]) {
        case 0: {
          std::cout << "Warning: not implemented parsing for type \"" << type
                    << "\", skipping " + data.first;
        }
        case 1: { // int16
          try {
            int16_t num = data.second.get<int16_t>();
            std::cout << data.first << ": " + std::to_string(num) << std::endl;
            metadata.push_back(std::make_pair(data.first, std::to_string(num)));
          } catch (const sdbus::Error &e) {
            std::cout << std::string("Error while trying to fetch int16: ") +
                             e.what();
            std::cout << "Received type: \"" + type +
                             "\" while \"n\" expected.";
            break;
          }

          break;
        }
        case 2: { // uint16
          try {
            uint16_t num = data.second.get<uint16_t>();
            std::cout << data.first << ": " << std::to_string(num) << std::endl;
            metadata.push_back(std::make_pair(data.first, std::to_string(num)));
          } catch (const sdbus::Error &e) {
            std::cout << std::string("Error while trying to fetch uint16: ") +
                             e.what();
            std::cout << "Received type: \"" + type +
                             "\" while \"q\" expected.";
            break;
          }

          break;
        }
        case 3: { // int32
          try {
            int32_t num = data.second.get<int32_t>();
            std::cout << data.first + ": " + std::to_string(num) << std::endl;
            metadata.push_back(std::make_pair(data.first, std::to_string(num)));
          } catch (const sdbus::Error &e) {
            std::cout << std::string("Error while trying to fetch int32: ") +
                             e.what();
            std::cout << "Received type: \"" + type +
                             "\" while \"i\" expected.";
            break;
          }

          break;
        }
        case 4: { // uint32
          try {
            uint32_t num = data.second.get<uint32_t>();
            std::cout << data.first + ": " + std::to_string(num) << std::endl;
            metadata.push_back(std::make_pair(data.first, std::to_string(num)));
          } catch (const sdbus::Error &e) {
            std::cout << std::string("Error while trying to fetch uint32: ") +
                             e.what();
            std::cout << "Received type: \"" + type +
                             "\" while \"i\" expected.";
            break;
          }

          break;
        }
        case 5: { // int64
          try {
            int64_t num = data.second.get<int64_t>();
            std::cout << data.first + ": " + std::to_string(num) << std::endl;
            metadata.push_back(std::make_pair(data.first, std::to_string(num)));
          } catch (const sdbus::Error &e) {
            std::cout << std::string("Error while trying to fetch int64: ") +
                             e.what();
            std::cout << "Received type: \"" + type +
                             "\" while \"x\" expected.";
            break;
          }

          break;
        }
        case 6: { // uint64
          try {
            uint64_t num = data.second.get<uint64_t>();
            std::cout << data.first + ": " + std::to_string(num) << std::endl;
            metadata.push_back(std::make_pair(data.first, std::to_string(num)));
          } catch (const sdbus::Error &e) {
            std::cout << std::string("Error while trying to fetch uint64: ") +
                             e.what();
            std::cout << "Received type: \"" + type +
                             "\" while \"d\" expected.";
            break;
          }

          break;
        }
        case 7: { // double
          try {
            double num = data.second.get<double>();
            std::cout << data.first + ": " + std::to_string(num) << std::endl;
            metadata.push_back(std::make_pair(data.first, std::to_string(num)));
          } catch (const sdbus::Error &e) {
            std::cout << std::string("Error while trying to fetch double: ") +
                             e.what();
            std::cout << "Received type: \"" + type +
                             "\" while \"t\" expected.";
            break;
          }

          break;
        }
        case 8: { // string
          try {
            std::string str = data.second.get<std::string>();
            std::cout << data.first + ": " + str << std::endl;
            metadata.push_back(std::make_pair(data.first, str));
          } catch (const sdbus::Error &e) {
            std::cout << std::string("Error while trying to fetch string: ") +
                             e.what();
            std::cout << "Received type: \"" + type +
                             "\" while \"s\" expected.";
            break;
          }

          break;
        }
        case 9: { // object path
          try {
            std::string path = data.second.get<sdbus::ObjectPath>();
            std::cout << data.first + ": " + path << std::endl;
            metadata.push_back(std::make_pair(data.first, path));
          } catch (const sdbus::Error &e) {
            std::cout << std::string(
                             "Error while trying to fetch object path: ") +
                             e.what();
            std::cout << "Received type: \"" + type +
                             "\" while \"o\" expected.";
            break;
          }
          break;
        }
        case 10: { // boolean
          try {
            bool boolean = data.second.get<bool>();
            std::cout << data.first + ": " << boolean << std::endl;

            metadata.push_back(
                std::make_pair(data.first, boolean ? "true" : "false"));
          } catch (const sdbus::Error &e) {
            std::cout << std::string("Error while trying to fetch boolean: ") +
                             e.what();
            std::cout << "Received type: \"" + type +
                             "\" while \"b\" expected.";
            break;
          }
          break;
        }
        case 11: { // array of strings
          try {
            std::vector<std::string> arr =
                data.second.get<std::vector<std::string>>();
            for (auto &entry : arr) {
              std::cout << data.first + ": " + entry << std::endl;
              metadata.push_back(std::make_pair(data.first, entry));
            }
          } catch (const sdbus::Error &e) {
            std::cout << std::string(
                             "Error while trying to fetch array of strings: ") +
                             e.what();
            std::cout << "Received type: \"" + type +
                             "\" while \"b\" expected.";
            break;
          }
          break;
        }
        default: {
          std::cout << "Got not implemented data type: " + type +
                           ", skipping " + data.first;
          break;
        }
        }
      }

      for (auto &info : metadata) {
        if (info.first == "xesam:artist") { // get new artist
          std::string new_artist = info.second;
          std::cout << "New artist: " << new_artist << std::endl;
          //   if (m_song_artist != new_artist) {
          //     m_song_artist = info.second;
          //     notify_observers_song_artist_changed(); // notify that artist
          //                                             // changed
          //   }
        } else if (info.first == "xesam:title") { // get new title
          std::string new_title = info.second;
          std::cout << "New title: " << new_title << std::endl;
          //   if (m_song_title != new_title) {
          //     m_song_title = info.second;
          //     notify_observers_song_title_changed(); // notify that title
          //                                            // changed
          //   }
        }
      }
      return;
    }
    case 4: { // PlaybackStatus
      std::cout << "PlaybackStatus property changed, new value: " +
                       prop.second.get<std::string>();
      bool new_is_playing = prop.second.get<std::string>() ==
                            "Playing"; // write new playback status
      //   if (m_is_playing != new_is_playing) {
      //     m_is_playing = new_is_playing;
      //     notify_observers_is_playing_changed(); // notify that playback
      //     status
      //                                            // changed
      //   }
      return;
    }
    }
  }
}

void DBusListener::on_name_owner_changed(sdbus::Signal &signal) {
  std::string name, old_owner, new_owner;
  signal >> name >> old_owner >> new_owner;

  if (name == "org.mpris.MediaPlayer2.spotify") {
    if (new_owner.empty()) {
      std::cout << "org.mpris.MediaPlayer2.spotify has been removed"
                << std::endl;
    } else {
      std::cout << "org.mpris.MediaPlayer2.spotify has been created"
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