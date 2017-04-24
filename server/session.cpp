#include "stdafx.h"
#include "session.h"
#include "chat_server.h"

Session::Session(int session_id, boost::asio::io_service& io_service, ChatServer* server)
  : socket_(io_service)
  , session_id_(session_id)
  , server_(server)
{
}

Session::~Session()
{
  while (send_data_queue_.empty() == false) {
    delete[] send_data_queue_.front();
    send_data_queue_.pop_front();
  }
}

void Session::Init()
{
  packet_buffer_mark_ = 0;
}


void Session::PostReceive()
{
  socket_.async_read_some(
    boost::asio::buffer(recv_buffer_),
    boost::bind(
      &Session::HandleRead, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
    )
  );
}

void Session::PostSend(const bool immediately, const int size, char* data)
{
  char* send_data = nullptr;

  if (immediately == false) {
    // 즉시 전송하지 말아라?
    send_data = new char[size];
    memcpy(send_data, data, size);
    send_data_queue_.push_back(send_data); // 큐잉했다가 보내라는 건가?
    return; // 여기서 리턴해야하지 않나?... -_-?
  }
  else {
    // 즉시 전송해라?
    send_data = data;
  }

  if (immediately == false && send_data_queue_.size() > 1) {
    // 즉시 전송하지 않아야 하고, 큐에 쌓인게 있다. 
    return;
  }

  // 실제 전송 (비동기라 보내졌다고 확신은 못할듯?)
  boost::asio::async_write(
    socket_, boost::asio::buffer(send_data, size),
    boost::bind(
      &Session::HandleWrite, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
    )
  );
}


void Session::HandleWrite(const boost::system::error_code& error, size_t bytes_transferred)
{
  delete[] send_data_queue_.front();
  send_data_queue_.front();

  if (send_data_queue_.empty() == false)
  {
    auto data = send_data_queue_.front();
    PACKET_HEADER* header = (PACKET_HEADER*)data;
    PostSend(true, header->size, data);
  }
}

void Session::HandleRead(const boost::system::error_code& error, size_t byte_transferred)
{
  if (error)
  {
    if (error == boost::asio::error::eof)
    {
      std::cout << "client disconnect" << std::endl;
    }
    else
    {
      std::cout << "error no : " << error.value() << " error mssage : " << error.message() << std::endl;
    }

    server_->CloseSession(session_id_);
  }
  else
  {
    memcpy(&packet_buffer_[packet_buffer_mark_], recv_buffer_.data(), byte_transferred);

    int packet_data = packet_buffer_mark_ + byte_transferred;
    auto read_data = 0;

    while (packet_data > 0) {
      if (packet_data < sizeof(PACKET_HEADER))
      {
        break;
      }

      auto header = (PACKET_HEADER*)packet_buffer_[read_data];

      if (header->size <= packet_data) {
        server_->ProcessPacket(session_id_, &packet_buffer_[read_data]);

        packet_data -= header->size;
        read_data += header->size;
      }
      else
      {
        break;
      }
    }

    if (packet_data > 0) {
      char temp_buffer[MAX_RECEIVE_BUFFER_LEN] = { 0, };
      memcpy(&temp_buffer[0], &packet_buffer_[read_data], packet_data);
      memcpy(&packet_buffer_[0], &temp_buffer[0], packet_data);

    }

    packet_buffer_mark_ = packet_data;

    PostReceive();
  }
}