cmake_minimum_required(VERSION 3.0.0)
project(raspberrypi-spotify VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

find_package(sdbus-c++ REQUIRED)
find_package(websocketpp REQUIRED)

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

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)