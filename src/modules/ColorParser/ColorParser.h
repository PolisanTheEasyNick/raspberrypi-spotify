#ifndef COLORPARSER
#define COLORPARSER

#ifdef PILED
//------------
#include <cstdint>
#include <curl/curl.h>

#include <string>

using namespace std;

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

void calculateAverageColor(unsigned char *data, int width, int height,
                           int channels, int &avgRed, int &avgGreen,
                           int &avgBlue);

void requestParseImage(const std::string &url);

// PiLED section
std::string hmac_sha256(const std::string &secret, const std::string &data);
void send_tcp_packet(const std::string &host, int port,
                     const std::string &data);
void send_color_request(uint8_t red, uint8_t green, uint8_t blue,
                        uint8_t duration, uint8_t steps);
//-----------
#endif

#endif