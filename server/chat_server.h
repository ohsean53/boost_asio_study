#pragma once

#include "stdafx.h"

#include "session.h"
#include "define.h"


class ChatServer
{
public:
  ChatServer(boost::asio::io_service& io_service);
  ~ChatServer();

  void Init(const int maxSessionCount);
  void Start();
  void CloseSession(const int sessionID);
  void ProcessPacket(const int seesionID, const char* data);

private:
  int seq_number_;
  bool is_accepting_;

  boost::asio::ip::tcp::acceptor acceptor_;
  std::vector< Session*> session_list_;
  std::deque <int> session_queue_;

  bool PostAccept();
  void HandleAccept(Session* session, const boost::system::error_code& error);
};

