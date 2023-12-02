#include "minipost.hpp"

httpRequest::httpRequest() {
#ifdef __WIN32__
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    logging::error("Error to initialize winsock library.");
  }
#endif
};

bool httpRequest::close_connection(SocketType connection) {
#ifdef __WIN32__
  closesocket(connection);
#else
  // close(connection);
#endif
  logging::info("Conection closed");
  return true;
};

bool httpRequest::send_data(SocketType connection, const char *data) {
  if (send(connection, data, strlen(data), 0) == Socket_error) {
    logging::error("send() SOCKET ERROR");
    // Al finalizar, verifica si hubo un error
    get_last_error();
    this->close_connection(connection);
    return 1;
  } else {
    return true;
  }
};

/* http
std::string httpRequest::resolveDomain(const char *domain) {
#ifdef __WIN32__
  struct hostent *remoteHost;
  struct in_addr addr;
  std::string ip_address = "";
  remoteHost = gethostbyname(domain);
  int i = 0;
  logging::info("Solving domain %s...", domain);
  if (remoteHost->h_addrtype == AF_INET) {
    while (remoteHost->h_addr_list[i] != 0) {
      addr.s_addr = *(u_long *)remoteHost->h_addr_list[i++];
      ip_address.assign(inet_ntoa(addr));
    }
  }
  logging::success("Solved: %s --> %s", domain, ip_address);
  return ip_address;
#else
  return domain;
#endif
}*/

Response httpRequest::post(
        const char* ipaddr, 
        const int16_t port,
        const char* endpoint, 
        json payload,
        const std::function<bool(std::string chunck, const CallbackBus *bus)> 
        &reader_callback = nullptr, 
        const CallbackBus *bus=nullptr)
{
  if (!debug) {
    logging::disable_msg();
  }
  logging::info("Connecting to %s to port %d", ipaddr,
                port);
  logging::info("Sending:", payload);
  SocketType connection =
    connect_to(ipaddr, port);
  logging::info("Socket status: %d", connection);
  Response response;
  
  logging::info("Sending POST request");

  std::string request_body = "POST " + std::string(endpoint)  + " HTTP/1.1\r\n";
  request_body += "Host: \r\n";
  request_body += "Accept: text/event-stream\r\n";
  request_body += "Content-Type: application/json\r\n";
  request_body += "Content-Length: " + std::to_string(payload.size()) + "\r\n";
  request_body += "Connection: keep-alive\r\n";
  request_body += "Sec-Fetch-Dest: empty\r\n";
  request_body += "Sec-Fetch-Mode: cors\r\n";
  request_body += "\r\n";
  request_body += payload;

  logging::info(request_body.c_str());

  if (!this->send_data(connection, request_body.c_str())) {
    std::cout << "Error sending request." << std::endl;
    response.Status = 400;
    return response;
  }

  char buffer[BUFFER_SIZE];
  int bytesRead;
  logging::info("Waiting recv() answer");
  while ((bytesRead = recv(connection, buffer, sizeof(buffer), 0)) > 0) {
    if (bytesRead == 0) break;

    std::string bufferStr(buffer);

    size_t httpPos = bufferStr.find("HTTP");
    if (httpPos != std::string::npos) {
      response.Status = std::stoi(bufferStr.substr(9, 3));
    };

    response.body.assign(buffer, bytesRead);

    if (reader_callback != nullptr) {
      if (!reader_callback(response.body, bus)) break;
    }
  };

  if (bytesRead == Socket_error) logging::error("Error receiving data.");

  return response;
};

SocketType httpRequest::connect_to(const char *ipaddr, int16_t port) {
#ifdef __WIN32__
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    logging::error("Error initializing winsock.");
    return 1;
  }
#endif

  SocketType clientSocket = socket(AF_INET, SOCK_STREAM, 0);

  if (clientSocket == Invalid_socket) {
    logging::error("Error to create a socket.");
    clean_up();
    return 1;
  }

  sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(port);
  serverAddress.sin_addr.s_addr = inet_addr(ipaddr);

  if (connect(clientSocket, (sockaddr *)&serverAddress,
              sizeof(serverAddress)) == Socket_error) {
    logging::error("Error connecting to the server.");
    get_last_error();
    close_connection(clientSocket);
    return -1;
  }
  logging::success("Socket created.");
  return clientSocket;
}

int httpRequest::get_last_error() {
#ifdef __WIN32__
  int errorCode = WSAGetLastError();
  if (errorCode != 0) {
    char errorBuffer[256];
    if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorCode, 0,
                      errorBuffer, sizeof(errorBuffer), NULL) != 0) {
      logging::error("Error in Winsock:\nCode: %d \nMsg:%s", errorCode,
                     errorBuffer);
    } else {
      logging::error(
        "Error in Winsock with Code %d (Not possible get code description).",
        errorCode);
    }
  }
  return errorCode;
#else
  perror("Error caused by ");
  return 1;
#endif
}

void httpRequest::clean_up() {
#ifdef __WIN32__
  WSACleanup();
#endif
}

httpRequest::~httpRequest() { clean_up(); }