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

namespace std {
template <> struct hash<Color> {
  size_t operator()(const Color &c) const {
    return hash<unsigned char>()(c.r) ^ (hash<unsigned char>()(c.g) << 8) ^
           (hash<unsigned char>()(c.b) << 16);
  }
};
} // namespace std

void adjustBrightness(int &red, int &green, int &blue, int targetBrightness) {
  int currentBrightness = (red + green + blue) / 3;
  if (currentBrightness < targetBrightness) {

    double factor = static_cast<double>(targetBrightness) / currentBrightness;
    red = std::min(static_cast<int>(red * factor), 255);
    green = std::min(static_cast<int>(green * factor), 255);
    blue = std::min(static_cast<int>(blue * factor), 255);
  }
}

void calculateAccentColorWithKMeansAndBlurring(
    unsigned char *data, int width, int height, int channels, int &accentRed,
    int &accentGreen, int &accentBlue, int blurKernelSize = 51, int K = 3) {
  cv::Mat image(height, width, CV_8UC3, data);

  cv::Mat blurredImage;
  cv::GaussianBlur(image, blurredImage,
                   cv::Size(blurKernelSize, blurKernelSize), 0);

  cv::Mat reshapedImage =
      blurredImage.reshape(1, blurredImage.rows * blurredImage.cols);

  reshapedImage.convertTo(reshapedImage, CV_32F);

  cv::Mat labels, centers;
  cv::kmeans(reshapedImage, K, labels,
             cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT,
                              10, 1.0),
             3, cv::KMEANS_PP_CENTERS, centers);

  centers.convertTo(centers, CV_8U);

  cv::Mat clusteredImage = centers.reshape(3, centers.rows);

  std::vector<int> clusterCounts(K, 0);
  for (int i = 0; i < labels.rows; ++i) {
    clusterCounts[labels.at<int>(i)]++;
  }

  int maxIdx = std::distance(
      clusterCounts.begin(),
      std::max_element(clusterCounts.begin(), clusterCounts.end()));
  cv::Vec3b dominantColor = clusteredImage.at<cv::Vec3b>(maxIdx);

  accentRed = dominantColor[0];
  accentGreen = dominantColor[1];
  accentBlue = dominantColor[2];
}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

bool downloadImage(const std::string &url, std::string &imageData) {
  std::cout << "Starting downloading image!" << std::endl;
  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();
  if (!curl) {
    std::cerr << "Failed to initialize CURL" << std::endl;
    return false;
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &imageData);
  res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    std::cerr << "Failed to download image: " << curl_easy_strerror(res)
              << std::endl;
    curl_easy_cleanup(curl);
    return false;
  }

  curl_easy_cleanup(curl);
  return true;
}
void calculateAverageColor(unsigned char *data, int width, int height,
                           int channels, int &avgRed, int &avgGreen,
                           int &avgBlue) {
  long long totalRed = 0, totalGreen = 0, totalBlue = 0;
  int pixelCount = width * height;

  for (int i = 0; i < pixelCount; ++i) {
    int index = i * channels;
    totalRed += data[index];
    totalGreen += data[index + 1];
    totalBlue += data[index + 2];
  }

  avgRed = totalRed / pixelCount;
  avgGreen = totalGreen / pixelCount;
  avgBlue = totalBlue / pixelCount;
}

void requestParseImage(const std::string &url) {
  std::cout << "Started parsing image..." << std::endl;
  std::string imageData;

  if (!downloadImage(url, imageData)) {
    std::cerr << "Failed to download image." << std::endl;
    return;
  }

  int width, height, channels;
  unsigned char *imgData =
      stbi_load_from_memory((unsigned char *)imageData.data(), imageData.size(),
                            &width, &height, &channels, 3);
  if (!imgData) {
    std::cerr << "Failed to load image!" << std::endl;
    return;
  }

  int avgRed = 0, avgGreen = 0, avgBlue = 0;
  calculateAccentColorWithKMeansAndBlurring(imgData, width, height, channels,
                                            avgRed, avgGreen, avgBlue);

  int brightness = (avgRed + avgGreen + avgBlue) / 3;

  int minChannel = std::min({avgRed, avgGreen, avgBlue});
  int maxChannel = std::max({avgRed, avgGreen, avgBlue});
  int diff = maxChannel - minChannel;

  if (brightness < 50) {

    std::cout << "Color is uniformly dark, using default color." << std::endl;
    avgRed = 209;
    avgGreen = 0;
    avgBlue = 255;

  } else {
    std::cout << "Color is bright enough." << std::endl;
  }

  stbi_image_free(imgData);

  std::cout << "Red: " << avgRed << ", Green: " << avgGreen
            << ", Blue: " << avgBlue << std::endl;
  send_color_request(avgRed, avgGreen, avgBlue, 3, 0);
  return;
}

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
