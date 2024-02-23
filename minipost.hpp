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
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    typedef int32_t SocketType;
    #define Socket_error -1
    #define Invalid_socket -1
#endif

#include "logging.hpp"
#include "terminal.hpp"

#define BUFFER_SIZE 50000 // 50kb

// JSON Type
typedef std::string json;

struct Response {
  int16_t Status = -1;
  std::string body;
};

struct CallbackBus {
  std::string buffer;
  bool stream;
};

class httpRequest {
public:
  httpRequest();
  // post http request by callback for data stream
  Response post(const char* ipaddr, 
                const int16_t port, 
                const char* endpoint,
                json payload,
                const std::function<bool(std::string chunck, const CallbackBus *bus)> &reader_callback, 
                const CallbackBus *bus);  

  // TODO: socket (separate it)
  bool debug = false;
  SocketType  connect_to(const char* ipaddr, int16_t port);
  bool        send_data(SocketType connection, const char* data);
  bool        close_connection(SocketType connection);
  int         get_last_error();
  void        clean_up();

  ~httpRequest();
};