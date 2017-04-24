#pragma once

#include <deque>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "define.h"

class ChatServer;

class Session
{
public:
  Session(int sessionID, boost::asio::io_service& io_service, ChatServer* server);
  ~Session();


  int GetSessionID() { return session_id_; }

  boost::asio::ip::tcp::socket& GetSocket() { return socket_; }

  void Init();
  void PostReceive();
  void PostSend(const bool immediately, const int size, char* data);

  void SetName(const char* name) {}
  const char* GetName() { return name_.c_str(); }

private:

  void HandleWrite(const boost::system::error_code& error, size_t bytesTransferred);
  void HandleRead(const boost::system::error_code& error, size_t bytesTransferred);

  int session_id_;
  boost::asio::ip::tcp::socket socket_;
  std::array<char, MAX_RECEIVE_BUFFER_LEN> recv_buffer_;

  int packet_buffer_mark_;
  char packet_buffer_[MAX_RECEIVE_BUFFER_LEN * 2];
  std::deque<char*> send_data_queue_;
  std::string name_;
  ChatServer* server_;
};

