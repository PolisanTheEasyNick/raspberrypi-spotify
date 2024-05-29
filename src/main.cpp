#include "modules/DBusListener/DBusListener.h"
#include "modules/WS-server/WSServer.h"
#include <csignal>
#include <iostream>
#include <thread>

bool running = true;

void signal_handler(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    std::cout << "[main] SIGINT or SIGTERM captured" << std::endl;
    running = false;
  }
}

int main(int, char **) {
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  DBusListener dbusListener;
  WSServer wsServer;

  dbusListener.add_observer(&wsServer);
  dbusListener.getSpotifyInfo();
  std::thread ws_thread([&wsServer] { wsServer.run(4831); });
  while (running) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  wsServer.stop();

  ws_thread.join();

  std::cout << "See you next time!" << std::endl;
  return 0;
}