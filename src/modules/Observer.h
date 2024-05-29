#ifndef OBSERVER_H
#define OBSERVER_H

#include <string>

class Observer {
public:
  virtual void on_update(const std::string &title, const std::string &artist,
                         const std::string &artUrl) = 0;
  virtual ~Observer() = default;
};

#endif // OBSERVER_H
