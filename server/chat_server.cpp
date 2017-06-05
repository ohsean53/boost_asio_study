#include "stdafx.h"
#include "chat_server.h"
#include "../proto/protocol.pb.h"
#include "util.h"

ChatServer::ChatServer(boost::asio::io_service & io_service)
  : acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT_NUMBER))
{
  is_accepting_ = false;
}

ChatServer::~ChatServer()
{
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
    std::cout << __FUNCTION__ << " session id initialize : " << i << std::endl;
  }
}

void ChatServer::Start()
{
  std::cout << __FUNCTION__ << " server start...." << std::endl;
  PostAccept(); // 서버시작시에 제일 먼저 호출
}

void ChatServer::CloseSession(const int session_id)
{
  std::cout << __FUNCTION__ << "client connect close, session_id : " << session_id << std::endl;

  session_list_[session_id]->GetSocket().close();
  session_queue_.push_back(session_id);
  if (is_accepting_ == false)
  {
    PostAccept();
  }
}

void ChatServer::ProcessPacket(const int session_id, const char* data, int msg_size)
{
  std::cout << std::endl;
  // 사이즈를 제외한 바디메시지만 와야함
  gs_protocol::Message req;
  auto is_parse_success = req.ParsePartialFromArray(data, msg_size);
  if (is_parse_success == false) {
    std::cout << __FUNCTION__  << " parse fail T.T" << std::endl;
  }

  auto msg_type = req.payload_case();

  switch (msg_type)
  {
  case gs_protocol::Message::kReqLogin:
  {
    auto req_login = req.req_login();
    std::cout << __FUNCTION__ << " req_login parse success" << std::endl;
    std::cout << __FUNCTION__ << " user_id : " << req_login.userid() << std::endl;

    session_list_[session_id]->SetUserID(req_login.userid());
    auto user_id = session_list_[session_id]->GetUserID();
    std::cout << __FUNCTION__ << " client login success, id : " << user_id << std::endl;

    gs_protocol::Message res;
    auto res_login = res.mutable_res_login();
    res_login->set_userid(user_id);
    res_login->set_result(1);

    session_list_[session_id]->PostWrite(false, SerializePBMessage(res));
  }
  break;

  case gs_protocol::Message::kReqCreate:
    break;
  case gs_protocol::Message::kReqJoin:
    break;
  case gs_protocol::Message::kReqAction1:
  {
    auto req_action1 = req.req_action1();
    std::cout << __FUNCTION__ << " req_action1 parse success" << std::endl;
    std::cout << __FUNCTION__ << " user_id : " << req_action1.userid() << std::endl;

    gs_protocol::Message notify;
    auto notify_action1 = notify.mutable_notify_action1();
    notify_action1->set_userid(req_action1.userid());
   
    for (auto& session : session_list_)
    {
      if (session_id == session->GetSessionID()) {
        continue;
      }
      if (session->GetSocket().is_open()) {
        session->PostWrite(false, SerializePBMessage(notify));
      }
    }
  }
    break;
  case gs_protocol::Message::kReqQuit:
    break;
  case gs_protocol::Message::kReqRoomList:
    break;
  case gs_protocol::Message::kResLogin:
    break;
  case gs_protocol::Message::kResCreate:
    break;
  case gs_protocol::Message::kResJoin:
    break;
  case gs_protocol::Message::kResAction1:
    break;
  case gs_protocol::Message::kResQuit:
    break;
  case gs_protocol::Message::kResRoomList:
    break;
  case gs_protocol::Message::kNotifyJoin:
    break;
  case gs_protocol::Message::kNotifyAction1:
  {
    auto notify_action1 = req.notify_action1();
    std::cout << __FUNCTION__ << "notify_action1 parse success" << std::endl;
    std::cout << __FUNCTION__ << "from user_id : " << notify_action1.userid() << std::endl;
  }
    break;
  case gs_protocol::Message::kNotifyQuit:
    break;
  default:
    std::cout << "not defined!" << std::endl;
    break;
  }
}

bool ChatServer::PostAccept()
{
  if (session_queue_.empty())
  {
    is_accepting_ = false;
    std::cout << __FUNCTION__ << " session_queue is empty" << std::endl;
    return false;
  }

  is_accepting_ = true;
  int session_id = session_queue_.front();
  session_queue_.pop_front();
  std::cout << __FUNCTION__ << " session_queue pop_front id : " << session_id << std::endl;

  acceptor_.async_accept(session_list_[session_id]->GetSocket(),
    boost::bind(
      &ChatServer::HandleAccept, this, session_list_[session_id], boost::asio::placeholders::error
    )
  );

  return true;
}

void ChatServer::HandleAccept(Session* session, const boost::system::error_code & error)
{
  // PostAccept 에서 async_accept 가 끝나고 호출당하는 메서드
  if (!error)
  {
    std::cout << "client connect success. session id : " << session->GetSessionID() << std::endl;

    session->Init();
    session->PostRead(); // 여기가 PostReceive 처음 호출하는 부분임
    PostAccept();
  }
  else
  {
    std::cout << __FUNCTION__ << " error no : " << error.value() << " error message : " << error.message() << std::endl;
  }
}
