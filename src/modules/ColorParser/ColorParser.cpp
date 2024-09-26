#include <cstdint>
#include <endian.h>
#ifdef PILED

#include <curl/curl.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "ColorParser.h"
#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <opencv2/opencv.hpp>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <random>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>

struct Color {
  unsigned char r, g, b;
  bool operator==(const Color &other) const {
    return r == other.r && g == other.g && b == other.b;
  }
};

// PiLED section

// function to send data via TCP
void send_tcp_packet(const std::string &host, int port,
                     const uint8_t data[54]) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    std::cerr << "Socket creation error" << std::endl;
    return;
  }

  sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  inet_pton(AF_INET, host.c_str(), &server_address.sin_addr);

  if (connect(sock, (struct sockaddr *)&server_address,
              sizeof(server_address)) < 0) {
    std::cerr << "Connection failed" << std::endl;
    close(sock);
    return;
  }

  send(sock, data, 54, 0);
  std::cout << "Data sent to " << host << ":" << port << std::endl;
  close(sock);
}

void send_color_request(uint8_t red, uint8_t green, uint8_t blue,
                        uint8_t duration, uint8_t steps) {
  using namespace std::chrono;

  // its so cursed bc contains C and C++ code mixed
  // i don't care, i just want it to work

  uint64_t current_time = __bswap_constant_64(time(NULL));

  // std::cout << "Current time: " << current_time << std::endl;

  // generate a random nonce
  std::random_device rd;
  std::mt19937_64 gen(rd());
  uint64_t rand = gen();
  uint64_t nonce = 0;
  std::memcpy(&nonce, &rand, 8);
  // create HEADER
  uint8_t header[18];
  std::memcpy(header, &current_time, 8); // Timestamp
  std::memcpy(header + 8, &nonce, 8);    // Nonce
  uint8_t version = 4;
  uint8_t OP = 0;
  header[16] = version;
  header[17] = OP;

  // create PAYLOAD
  uint8_t payload[5];
  payload[0] = red;
  payload[1] = green;
  payload[2] = blue;
  payload[3] = duration;
  payload[4] = steps;

  // combine HEADER and PAYLOAD
  uint8_t header_with_payload[23];
  std::memcpy(header_with_payload, header, 18);
  std::memcpy(header_with_payload + 18, payload, 5);

  // std::cout << "Header with payload:\n";

  // for (int i = 0; i < 23; i++) {
  //   printf("%02x", header_with_payload[i]);
  // }
  // printf("\n");
  // for (int i = 0; i < 23; i++) {
  //   printf("%02x ", header_with_payload[i]);
  // }
  // printf("\n");

  // compute HMAC-SHA256
  unsigned char GENERATED_HMAC[32];
  unsigned int hmac_len;
  std::string SHARED_KEY = "SHARED_SECRET";
  HMAC(EVP_sha256(), SHARED_KEY.c_str(), SHARED_KEY.size(), header_with_payload,
       23, GENERATED_HMAC, &hmac_len);
  // std::cout << "Timestamp: " << current_time << std::endl;
  // std::cout << "Nonce: " << nonce << std::endl;
  // std::cout << "Payload: ";
  printf("%x %x %x %x\n", red, green, blue, duration);
  // std::cout << "Generated HMAC:" << std::endl;
  for (size_t i = 0; i < 32; i++) {
    printf("%x ", GENERATED_HMAC[i]);
  }
  std::cout << std::endl;

  // create final TCP package
  uint8_t tcp_package[55];
  std::memcpy(tcp_package, header, 18);
  std::memcpy(tcp_package + 18, GENERATED_HMAC, 32);
  std::memcpy(tcp_package + 50, payload, 5);
  //  std::cout << "Generated TCP Package:\n";
  for (int i = 0; i < 54; i++) {
    printf("%x ", tcp_package[i]);
  }
  std::cout << std::endl;

  // send TCP package
  send_tcp_packet("192.168.0.5", 3384, tcp_package);
}

#endif
