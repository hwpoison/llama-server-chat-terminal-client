#include "minipost.hpp"

httpRequest::httpRequest() {
#ifdef __WIN32__
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    logging::error("Error to initialize winsock library.");
  }
#endif
};

bool httpRequest::closeConnection(SocketType connection) {
#ifdef __WIN32__
  closesocket(connection);
#else
  close(connection);
#endif
  logging::info("Conection closed");
  return true;
};

bool httpRequest::sendData(SocketType connection, const char *data) {
  if (send(connection, data, strlen(data), 0) == Socket_error) {
    logging::error("send() SOCKET ERROR");
    getLastError();
    this->closeConnection(connection);
    return 1;
  } else {
    return true;
  }
};

Response httpRequest::post(
        const char* ipaddr, 
        const int16_t port,
        const char* endpoint, 
        json payload,
        const std::function<bool(std::string chunck, const CallbackBus *bus)> &reader_callback = nullptr, 
        const CallbackBus *bus=nullptr)
{
  if (!debug) {
    logging::disable_msg();
  }

  Response response;
  logging::info("Connecting to %s to port %d", ipaddr,
                port);
  logging::info("Sending:", payload.c_str());
  SocketType connection =
    connectTo(ipaddr, port);
  logging::info("Socket status: %d", connection);
  
  
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

  if (!this->sendData(connection, request_body.c_str())) {
    std::cout << "Error sending request." << std::endl;
    response.Status = 400;
    return response;
  }

  char buffer[BUFFER_SIZE];
  int bytesRead;
  bool streaming=true;
  logging::info("Waiting recv() answer");

  std::string accumulatedBuffer;
  
  while (streaming
    and (bytesRead = recv(connection, buffer, sizeof(buffer), 0)) > 0) {
      if (bytesRead == 0) break;
      accumulatedBuffer.append(buffer, bytesRead);
      
      // Get HTTP code
      size_t httpPos = accumulatedBuffer.find("HTTP");
      if (httpPos != std::string::npos) {
        response.Status = std::stoi(accumulatedBuffer.substr(9, 3));
      };

      // Iter stream content line by line
      size_t lineEndPos;
      while ((lineEndPos = accumulatedBuffer.find('\n')) != std::string::npos) {
          std::string line = accumulatedBuffer.substr(0, lineEndPos);
          accumulatedBuffer.erase(0, lineEndPos + 1);
          if (reader_callback != nullptr) {
              if (!reader_callback(line, bus)) streaming=false;
          }
      }

      // remain data
      if (!accumulatedBuffer.empty() && streaming) {
          if (reader_callback != nullptr) {
              reader_callback(accumulatedBuffer, bus);
          }
      }
  };

  if (bytesRead == Socket_error) logging::error("Error receiving data.");

  closeConnection(connection);
  return response;
}

SocketType httpRequest::connectTo(const char *ipaddr, int16_t port) {
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
    cleanUp();
    return 1;
  }

  sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(port);
  serverAddress.sin_addr.s_addr = inet_addr(ipaddr);

  if (connect(clientSocket, (sockaddr *)&serverAddress,
              sizeof(serverAddress)) == Socket_error) {
    logging::error("Error connecting to the server.");
    getLastError();
    closeConnection(clientSocket);
    return -1;
  }
  logging::success("Socket created.");
  return clientSocket;
}

int httpRequest::getLastError() {
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
  perror("Error caused by "); // ??
  return 1;
#endif
}

void httpRequest::cleanUp() {
#ifdef __WIN32__
  WSACleanup();
#endif
}

httpRequest::~httpRequest() { cleanUp(); }
