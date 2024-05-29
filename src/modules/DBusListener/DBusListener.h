#ifndef DBUSLISTENER_H
#define DBUSLISTENER_H

#include <sdbus-c++/Message.h>
#include <sdbus-c++/sdbus-c++.h>

class DBusListener {
private:
  std::unique_ptr<sdbus::IConnection> m_dbus_conn;
  std::unique_ptr<sdbus::IProxy> m_proxy_signal;
  void on_properties_changed(sdbus::Signal &signal);
  void on_name_owner_changed(sdbus::Signal &signal);
  void subscribe();
  void unsubscribe();

public:
  DBusListener();
};

#endif // DBUSLISTENER.H