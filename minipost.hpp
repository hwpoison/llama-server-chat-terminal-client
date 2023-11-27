#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>

#ifdef __WIN32__
#include <winsock2.h>
#include <windows.h>
typedef SOCKET SocketType;
#define Socket_error SOCKET_ERROR
#define Invalid_socket INVALID_SOCKET
#endif

#include "logging.hpp"

#define BUFFER_SIZE 500000

// JSON Type
typedef std::string json;

struct Response {
  int16_t Status = -1;
  std::string body = "";
};

struct CallbackBus {
  std::string buffer;
  bool stream;
  bool stopCompletion;
};

class URLParser {
public:
  URLParser(std::string url) { parserURL(url); }

  std::string getDomain() { return domain; }

  int16_t getPort() { return port; }

  std::string getPath() { return path; }

  void parserURL(std::string& url) {
    size_t pos = url.find("://");
    if (pos != std::string::npos) {
      url = url.substr(pos + 3);
    }

    pos = url.find("/");
    if (pos != std::string::npos) {
      domain = url.substr(0, pos);
      path = url.substr(pos);
      pos = domain.find(":");
      if (pos != std::string::npos) {
        port = stoi(domain.substr(pos + 1));
        domain = domain.substr(0, pos);
      }
    }
  }

private:
  std::string domain;
  int16_t port = 80;  // http default port
  std::string path;
};

class httpRequest {
public:
  httpRequest();
  // http requests
  Response post(const std::string url, json payload,
                const std::function<bool(std::string chunck, const CallbackBus *bus)> &reader_callback, 
                const CallbackBus *bus);

  // TODO: socket (separate it)
  bool debug = false;
  SocketType  connect_to(const char* domain, int16_t port);
  bool        send_data(SocketType connection, const char* data);
  bool        close_connection(SocketType connection);
  std::string resolveDomain(const char* domain);
  int         get_last_error();
  void        clean_up();

  ~httpRequest();
};