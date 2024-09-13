cmake_minimum_required(VERSION 3.0.0)
project(raspberrypi-spotify VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

find_package(sdbus-c++ REQUIRED)
find_package(websocketpp REQUIRED)

add_definitions(-DPILED) #Check ColorParser and config it!
set(PILED 1)

if(PILED)
    find_package(CURL REQUIRED)
    find_package(OpenSSL REQUIRED)
    find_package(OpenCV 4.0.0 REQUIRED)
    include_directories("/usr/include/opencv4/")
endif()

if(PILED)

    add_executable(
        raspberrypi-spotify
        src/main.cpp
        src/modules/DBusListener/DBusListener.cpp
        src/modules/DBusListener/DBusListener.h
        src/modules/WS-server/WSServer.cpp
        src/modules/WS-server/WSServer.h
        src/modules/Observer.h
        src/modules/Subject.h
        src/modules/ColorParser/ColorParser.h
        src/modules/ColorParser/ColorParser.cpp
    )
    target_link_libraries(raspberrypi-spotify PRIVATE SDBusCpp::sdbus-c++ websocketpp::websocketpp CURL::libcurl OpenSSL::SSL)
else()
    add_executable(
        raspberrypi-spotify
        src/main.cpp
        src/modules/DBusListener/DBusListener.cpp
        src/modules/DBusListener/DBusListener.h
        src/modules/WS-server/WSServer.cpp
        src/modules/WS-server/WSServer.h
        src/modules/Observer.h
        src/modules/Subject.h
    )
    target_link_libraries(raspberrypi-spotify PRIVATE SDBusCpp::sdbus-c++ websocketpp::websocketpp)
endif()


install(TARGETS raspberrypi-spotify DESTINATION bin)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

if(EXISTS "/etc/systemd/user")
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/rasp-spot.service
        DESTINATION /etc/systemd/user)
    message(
        STATUS "To start the RaspberryPi Spotify service, run: systemctl --user start rasp-spot"
    )
    message(
        STATUS
        "To enable the RaspberryPi Spotify service at boot, run: systemctl --user enable rasp-spot"
    )
else()
    message(
        WARNING
        "The /etc/systemd/system directory does not exist. The RaspberryPi Spotify service file was not installed."
    )
endif()
