#include "WSServer.h"

WSServer::WSServer() {
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

void WSServer::on_open(websocketpp::connection_hdl hdl) {
  m_connections.insert(hdl);
}

void WSServer::on_close(websocketpp::connection_hdl hdl) {
  m_connections.erase(hdl);
}

void WSServer::send_update(const std::string &title, const std::string &artist,
                           const std::string &artUrl) {
  std::cout << "[WebSocket] Sending title: " << title << ", " << artist << ", "
            << artUrl << std::endl;
  std::string message = "{\"title\": " + title + ", \"artist\": " + artist +
                        "\"artURL\": " + artUrl + "\"}";
  for (auto hdl : m_connections) {
    m_server.send(hdl, message, websocketpp::frame::opcode::text);
  }
}

void WSServer::on_update(const std::string &title, const std::string &artist,
                         const std::string &artUrl) {
  std::cout << "[WebSocket] Got update!!!" << std::endl;
  send_update(title, artist, artUrl);
}