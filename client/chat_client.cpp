#pragma once

#include "chat_client.h"

ChatClient::ChatClient(boost::asio::io_service & io_service)
  : io_service_(io_service),
  socket_(io_service)
{
  is_login_ = false;
  // 크리티컬 섹션 초기화?
}

ChatClient::~ChatClient()
{
  // 락 시작

  while (send_data_queue_.empty() == false)
  {
    delete[] send_data_queue_.front();
    send_data_queue_.pop_front();
  }

  // 락 해제.. unique_lock?
}

bool ChatClient::IsConnecting()
{
  return socket_.is_open();
}

bool ChatClient::IsLoing()
{
  return is_login_;
}

void ChatClient::Login()
{
  is_login_ = true;
}

void ChatClient::Connect(boost::asio::ip::tcp::endpoint endpoint)
{
  packet_buffer_mark_ = 0;
  socket_.async_connect(endpoint,
    boost::bind(&ChatClient::HandleConnect, this, boost::asio::placeholders::error)
  );
}

void ChatClient::Close()
{
  if (socket_.is_open()) {
    socket_.close();
  }
}

void ChatClient::PostSend(const bool immediately, const int size, std::unique_ptr<std::vector<char>> data)
{
  std::unique_ptr<std::vector<char>> send_data = nullptr; // unique_ptr로 바꿀까

  // 락 시작

  if (immediately == false) {
    send_data = std::make_unique<std::vector<char>>(size);

  }

  // 락 해제
}

void ChatClient::HandleConnect(const boost::system::error_code & error)
{
}

void ChatClient::HandleWrite(const boost::system::error_code & error, size_t bytes_transferred)
{
}

void ChatClient::HandleRead(const boost::system::error_code & error, size_t bytes_transferred)
{
}

void ChatClient::ProcessPacket(const char * data)
{
}
