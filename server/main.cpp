#include "stdafx.h"
#include "chat_server.h"

const int MAX_SESSION_COUNT = 100;

int main()
{
  auto console = spdlog::stdout_color_mt(CONSOLE);
  spdlog::set_level(spdlog::level::trace); // 로그레벨
  SPDLOG_TRACE(console, "program start");
  console->trace("test");
  boost::asio::io_service io_service;
  ChatServer server(io_service);
  server.Init(MAX_SESSION_COUNT);
  server.Start();
  
  io_service.run();

  console->info("network connect close");
  SPDLOG_TRACE(console, "network connect close");
  return 0;
}
