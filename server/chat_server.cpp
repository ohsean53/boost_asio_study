#include "stdafx.h"
#include "chat_server.h"


ChatServer::ChatServer(boost::asio::io_service & io_service)
	: acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT_NUMBER))
{
	is_accepting_ = false;
}

ChatServer::~ChatServer()
{
	for (auto i = 0; i < session_list_.size(); ++i) {
		if ( session_list_[i]->socket_().isOpen() ) {
			session_list_[i]->socket_().close();
		}

		delete session_list_[i];
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
  std::cout << "서버 시작...." << std::endl;
  PostAccept();
}

void ChatServer::CloseSession(const int session_id)
{
  std::cout << "client connect close, session_id : " << session_id << std::endl;

  session_list_[session_id]->
}

void ChatServer::HandleAccept(Session * session, const boost::system::error_code & error)
{
	if (!error)
	{
		std::cout << "client connect success. session id : " << session->GetSessionID() << std::endl;
	}
}
