#ifndef SUBJECT_H
#define SUBJECT_H

#ifdef PILED
#include "ColorParser/ColorParser.h"
#endif
#include "Observer.h"
#include <algorithm>
#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
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
                        const std::string &album, const std::string &artUrl,
                        const bool &spotify_started, const bool &is_playing,
                        const bool &is_gamemode_running) {
    for (auto &observer : observers) {
      observer->on_update(title, artist, album, artUrl, spotify_started,
                          is_playing, is_gamemode_running);
    }
#ifdef PILED
    if (!is_gamemode_running) {
      std::cout << "Starting image thread" << std::endl;

      if (is_playing) {
        std::lock_guard<std::mutex> lock(thread_mutex);

        if (imageThread.joinable()) {
          stopRequested = true;
          imageThread.join();
          stopRequested = false;
        }

        imageThread = std::thread(requestParseImage, artUrl);
      } else {
        send_color_request(209, 0, 255, 3, 0);
      }
    }
#endif
  }

private:
  std::vector<Observer *> observers;
#ifdef PILED
  std::thread imageThread;
  std::mutex thread_mutex;
  static std::atomic<bool> stopRequested;
#endif
};

#endif // SUBJECT_H
