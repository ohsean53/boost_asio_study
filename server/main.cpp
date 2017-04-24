#include "stdafx.h"
#include "chat_server.h"

const int MAX_SESSION_COUNT = 100;

int main()
{
  boost::asio::io_service io_service;

  ChatServer server(io_service);
  server.Init(MAX_SESSION_COUNT);
  server.Start();

  io_service.run();

  std::cout << "network connect close" << std::endl;

  return 0;
}
