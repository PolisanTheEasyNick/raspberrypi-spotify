#include "modules/DBusListener/DBusListener.h"
#include "modules/WS-server/WSServer.h"

int main(int, char **) {
  DBusListener dbusListener;
  WSServer wsServer;

  dbusListener.add_observer(&wsServer);

  wsServer.run(4831);
}
