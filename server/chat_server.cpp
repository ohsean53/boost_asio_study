#include "stdafx.h"
#include "chat_server.h"


ChatServer::ChatServer(boost::asio::io_service & io_service)
  : acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT_NUMBER))
{
  is_accepting_ = false;
}

ChatServer::~ChatServer()
{
  /*for (auto i = 0; i < session_list_.size(); ++i) {
    if (session_list_[i]->GetSocket().is_open())
    {
      session_list_[i]->GetSocket().close();
    }

    delete session_list_[i];
  }*/

  for (auto& session : session_list_) {
    if (session->GetSocket().is_open()) {
      session->GetSocket().close();
    }
    delete session;
  }
}

void ChatServer::Init(const int maxSessionCount)
{
  for (auto i = 0; i < maxSessionCount; ++i) {
    Session* session = new Session(i, acceptor_.get_io_service(), this);
    session_list_.push_back(session);
    session_queue_.push_back(i);
  }
}

void ChatServer::Start()
{
  std::cout << "server start...." << std::endl;
  PostAccept();
}

void ChatServer::CloseSession(const int session_id)
{
  std::cout << "client connect close, session_id : " << session_id << std::endl;

  session_list_[session_id]->GetSocket().close();
  session_queue_.push_back(session_id);
  if (is_accepting_ == false)
  {
    PostAccept();
  }
}

void ChatServer::ProcessPacket(const int session_id, const char* data)
{
  PACKET_HEADER* header = (PACKET_HEADER*)data;
  switch (header->id)
  {
  case REQ_IN:
  {
    PKT_REQ_IN* packet = (PKT_REQ_IN*)data;
    session_list_[session_id]->SetName(packet->name);
    std::cout << "client login success, name : " << session_list_[session_id]->GetName() << std::endl;

    PKT_RES_IN send_packet;
    send_packet.Init();
    send_packet.is_success = true;

    session_list_[session_id]->PostSend(false, send_packet.size, (char*)&send_packet);

  }
  break;
  case REQ_CHAT:
  {
    PKT_REQ_CHAT* packet = (PKT_REQ_CHAT*)data;
    PKT_NOTICE_CHAT send_packet;
    send_packet.Init();
    strncpy_s(send_packet.name, MAX_NAME_LEN, session_list_[session_id]->GetName(), MAX_NAME_LEN - 1);
    strncpy_s(send_packet.message, MAX_MESSAGE_LEN, packet->message, MAX_MESSAGE_LEN - 1);

    for (auto& session : session_list_)
    {
      if (session->GetSocket().is_open()) {
        session->PostSend(false, send_packet.size, (char*)&send_packet);
      }
    }
  }
  default:
    break;
  }
}

bool ChatServer::PostAccept()
{
  if (session_queue_.empty())
  {
    is_accepting_ = false;
    return false;
  }

  is_accepting_ = true;
  int session_id = session_queue_.front();

  acceptor_.async_accept(session_list_[session_id]->GetSocket(),
    boost::bind(
      &ChatServer::HandleAccept, this, session_list_[session_id], boost::asio::placeholders::error
    )
  );

  return true;
}

void ChatServer::HandleAccept(Session * session, const boost::system::error_code & error)
{
  if (!error)
  {
    std::cout << "client connect success. session id : " << session->GetSessionID() << std::endl;

    session->Init();
    session->PostReceive();
    PostAccept();
  }
  else
  {
    std::cout << "error no : " << error.value() << " error message : " << error.message() << std::endl;
  }
}
