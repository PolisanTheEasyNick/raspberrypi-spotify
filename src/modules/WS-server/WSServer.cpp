#include "WSServer.h"

std::string escape_quotes(const std::string &input) {
  std::ostringstream escaped;
  for (char c : input) {
    if (c == '"') {
      escaped << "\\\"";
    } else {
      escaped << c;
    }
  }
  return escaped.str();
}

std::string create_json_message(const std::string &title,
                                const std::string &artist,
                                const std::string &artUrl, bool spotify_started,
                                bool isPlaying) {
  std::string escapedTitle = escape_quotes(title);
  std::string escapedArtist = escape_quotes(artist);

  std::string message =
      "{\"title\": \"" + escapedTitle + "\", \"artist\": \"" + escapedArtist +
      "\", \"artURL\": \"" + artUrl + "\", \"spotifyStarted\": \"" +
      (spotify_started ? "True" : "False") + "\", \"isPlaying\": \"" +
      (isPlaying ? "True" : "False") + "\"}";

  return message;
}

WSServer::WSServer() {
  m_server.set_reuse_addr(true);
  m_server.init_asio();

  m_server.set_access_channels(websocketpp::log::alevel::all);
  m_server.clear_access_channels(websocketpp::log::alevel::frame_payload);

  m_server.set_open_handler(
      bind(&WSServer::on_open, this, websocketpp::lib::placeholders::_1));
  m_server.set_close_handler(
      bind(&WSServer::on_close, this, websocketpp::lib::placeholders::_1));
}

void WSServer::run(uint16_t port) {
  m_server.listen(port);
  m_server.start_accept();
  try {
    std::cout << "[WebSocket] Started server on " << port << "!" << std::endl;
    m_server.run();
  } catch (const std::exception &e) {
    std::cerr << "[WebSocket] Error: " << e.what() << std::endl;
  }
}

void WSServer::stop() {
  std::cout << "[WebSocket] Stopping WebSocket server..." << std::endl;
  m_server.stop_listening();
  m_server.stop_perpetual();
  for (auto hdl : m_connections) {
    m_server.close(hdl, websocketpp::close::status::normal,
                   "Server shutting down");
  }
  std::cout << "Stopped!" << std::endl;
}

void WSServer::on_open(websocketpp::connection_hdl hdl) {
  m_connections.insert(hdl);

  m_server.send(hdl,
                create_json_message(m_title, m_artist, m_artURL,
                                    m_spotify_started, m_is_playing),
                websocketpp::frame::opcode::text);
}

void WSServer::on_close(websocketpp::connection_hdl hdl) {
  m_connections.erase(hdl);
}

void WSServer::send_update() {
  auto message = create_json_message(m_title, m_artist, m_artURL,
                                     m_spotify_started, m_is_playing);
  std::cout << "[WebSocket] Sending: " << message << std::endl;

  for (auto hdl : m_connections) {
    m_server.send(hdl, message, websocketpp::frame::opcode::text);
  }
}

void WSServer::on_update(const std::string &title, const std::string &artist,
                         const std::string &artUrl, const bool &spotify_started,
                         const bool &isPlaying) {
  std::cout << "[WebSocket] Got update!!!" << std::endl;
  m_title = title;
  m_artist = artist;
  m_artURL = artUrl;
  m_spotify_started = spotify_started;
  m_is_playing = isPlaying;
  send_update();
}