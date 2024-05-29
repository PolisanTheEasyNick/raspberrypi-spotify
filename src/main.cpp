#include "modules/DBusListener/DBusListener.h"
#include "modules/WS-server/WSServer.h"
#include <csignal>
#include <iostream>
#include <thread>

// Global flag for signal handling
bool running = true;

// Signal handler function
void signal_handler(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    running = false;
  }
}

int main(int, char **) {
  // Set up the signal handler
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  DBusListener dbusListener;
  WSServer wsServer;

  dbusListener.add_observer(&wsServer);

  std::thread ws_thread([&wsServer] { wsServer.run(4831); });

  // Main thread sleeps and periodically checks the running flag
  while (running) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  wsServer.stop();

  ws_thread.join();

  std::cout << "See you next time!" << std::endl;
  return 0;
}