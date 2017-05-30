#pragma once

#include <deque>

#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "define.h"
#include "../proto/protocol.pb.h"

class ChatServer;

class Session : public std::enable_shared_from_this<Session>
{
public:
  Session(int sessionID, boost::asio::io_service& io_service, ChatServer* server);
  ~Session();


  int GetSessionID() { return session_id_; }

  boost::asio::ip::tcp::socket& GetSocket() { return socket_; }

  void Init();
  void PostRead();
  void PostWrite(const bool immediately, std::shared_ptr<std::vector<char>> msg);

  void SetUserID(int64_t user_id) { user_id_ = user_id; }
  int64_t GetUserID() { return user_id_; }

private:

  void HandleRead(const boost::system::error_code& error, size_t bytesTransferred);

  int session_id_;
  boost::asio::ip::tcp::socket socket_;
  std::array<char, MAX_RECEIVE_BUFFER_LEN> recv_buffer_;

  int packet_buffer_mark_;
  char packet_buffer_[MAX_RECEIVE_BUFFER_LEN * 2];
  std::deque<std::shared_ptr<std::vector<char>>> send_msg_queue_;
  int64_t user_id_;
  ChatServer* server_;
};

