#include "stdafx.h"
#include "chat_server.h"
#include "../proto/protocol.pb.h"
#include "util.h"

ChatServer::ChatServer(boost::asio::io_service & io_service)
  : acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), PORT_NUMBER))
{
  is_accepting_ = false;
  logger_ = spdlog::get(CONSOLE);
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
    //logger::get(CONSOLE)->info("session id initialize {0}", i);
  }
}

void ChatServer::Start()
{
  logger_->info("server start..");
  PostAccept(); // 서버시작시에 제일 먼저 호출
}

void ChatServer::CloseSession(const int session_id)
{
  logger_->debug("client connect close, session_id : {0}", session_id);

  session_list_[session_id]->GetSocket().close();
  session_queue_.push_back(session_id);
  if (is_accepting_ == false)
  {
    PostAccept();
  }
}

void ChatServer::ProcessPacket(const int session_id, const char* data, int msg_size)
{
  // 사이즈를 제외한 바디메시지만 와야함
  gs_protocol::Message req;
  auto is_parse_success = req.ParsePartialFromArray(data, msg_size);
  if (is_parse_success == false) {
    logger_->error("fail, protobuf message parse");
  }

  auto msg_type = req.payload_case();

  switch (msg_type)
  {
  case gs_protocol::Message::kReqLogin:
  {
    auto req_login = req.req_login();

    logger_->debug("req_login parse user_id : {}", req_login.userid());

    session_list_[session_id]->SetUserID(req_login.userid());
    auto user_id = session_list_[session_id]->GetUserID();

    logger_->debug("client login success, user_id : {}", user_id);

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

    SPDLOG_TRACEF(logger_, "req_action1 parse success : {}", req_action1.userid());

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
    logger_->debug("notify_action1 parse success, from user_id : {}", notify_action1.userid());
  }
    break;
  case gs_protocol::Message::kNotifyQuit:
    break;
  default:
    logger_->error("not defined message type");
    break;
  }
}

bool ChatServer::PostAccept()
{
  if (session_queue_.empty())
  {
    is_accepting_ = false;
    logger_->error("fail post accept, session_queue is empty");
    return false;
  }

  is_accepting_ = true;
  int session_id = session_queue_.front();
  session_queue_.pop_front();
  logger_->debug("session_queue pop_front id : {}", session_id);

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
    logger_->info("client connect success. session id : {}", session->GetSessionID());
    session->Init();
    session->PostRead(); // 여기가 PostReceive 처음 호출하는 부분임
    PostAccept();
  }
  else
  {
    logger_->critical("error no : {0}, error message : {1}", error.value(), error.message());
  }
}
