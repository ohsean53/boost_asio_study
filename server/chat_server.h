#pragma once

#include "stdafx.h"

#include "session.h"
#include "../proto/protocol.pb.h"


class ChatServer
{
public:
  ChatServer(boost::asio::io_service& io_service);
  virtual ~ChatServer();

  void Init(const int maxSessionCount);
  void Start();
  void CloseSession(const int sessionID);
  void ProcessPacket(const int seesionID, const char* data, int msg_size);

private:
  int seq_number_;
  bool is_accepting_;

  boost::asio::ip::tcp::acceptor acceptor_;
  std::vector< Session*> session_list_;
  std::deque <int> session_queue_;

  bool PostAccept();
  void HandleAccept(Session* session, const boost::system::error_code& error);
};

