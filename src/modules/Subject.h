#ifndef SUBJECT_H
#define SUBJECT_H

#include "Observer.h"
#include <algorithm>
#include <vector>

class Subject {
public:
  void add_observer(Observer *observer) { observers.push_back(observer); }

  void remove_observer(Observer *observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer),
                    observers.end());
  }

protected:
  void notify_observers(const std::string &title, const std::string &artist,
                        const std::string &artUrl) {
    for (auto &observer : observers) {
      observer->on_update(title, artist, artUrl);
    }
  }

private:
  std::vector<Observer *> observers;
};

#endif // SUBJECT_H
