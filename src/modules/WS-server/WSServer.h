#ifndef WSSERVER_H
#define WSSERVER_H

#include "../Observer.h"
#include <cstdint>
#include <set>
#include <string>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;
typedef server::message_ptr message_ptr;

class WSServer : public Observer {
private:
  server m_server;
  std::set<websocketpp::connection_hdl,
           std::owner_less<websocketpp::connection_hdl>>
      m_connections;

  // last sent spotify related data
  std::string m_artURL;
  std::string m_artist;
  std::string m_title;
  bool m_spotify_started;
  bool m_is_playing;

public:
  WSServer();
  void run(uint16_t port);
  void stop();
  void on_open(websocketpp::connection_hdl hdl);
  void on_close(websocketpp::connection_hdl hdl);
  void send_update(const std::string &title, const std::string &artist,
                   const std::string &artUrl, const bool &spotify_started,
                   const bool &is_playing);

  // observer interface method
  void on_update(const std::string &title, const std::string &artist,
                 const std::string &artUrl, const bool &spotify_started,
                 const bool &is_playing) override;
};

#endif // WSSERVER.H