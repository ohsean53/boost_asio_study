#include "stdafx.h"
#include "Session.h"


Session::Session(int session_id, boost::asio::io_service& io_service, ChatServer* server)
	: socket_(io_service)
	, session_id_(session_id)
	, server_(server)
{
}

Session::~Session()
{
}

void Session::Init()
{
	packet_buffer_mark_ = 0;
}