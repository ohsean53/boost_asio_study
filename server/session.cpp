#include "stdafx.h"
#include "session.h"
#include "chat_server.h"
#include "util.h"
#include <algorithm>

template<typename T>
inline std::shared_ptr<T> MakeArray(int size)
{
  return std::shared_ptr<T>(new T[size], [](T *p) { delete[] p; });
}

Session::Session(int session_id, boost::asio::io_service& io_service, ChatServer* server)
  : socket_(io_service)
  , session_id_(session_id)
  , server_(server)
{
}

Session::~Session()
{
  while (send_msg_queue_.empty() == false) {
    {
      auto will_delete = std::move(send_msg_queue_.front());
    }
    send_msg_queue_.pop_front();
  }
}

void Session::Init()
{
  packet_buffer_mark_ = 0;
}


void Session::PostRead()
{
  socket_.async_read_some(
    boost::asio::buffer(recv_buffer_),
    boost::bind(
      &Session::HandleRead, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
    )
  );
}

// 이 메서드를 통해서 클라이언트에게 메시지를 보냄
void Session::PostWrite(const bool immediately, std::shared_ptr<std::vector<char>> msg)
{
  if (immediately == false) {
    // 즉시 전송하지 않고, 큐잉 함
    send_msg_queue_.push_back(msg);
  }

  if (immediately == false && send_msg_queue_.size() >= 2) {
    // 즉시 전송하지 않아야 하고, 큐에 쌓인게 2개 이상일 경우. 
    // 이전에 보내기 요청을 한 것이 완료되지 않았을 경우 곧바로 보내지 않고 보관함
    // 비동기 보내기를 할 때 실수하기 쉬운 것중의 하나가 보내기 요청을 한 후 데이터를
    // 바로 삭제하여 '보내기 실패' 가 발생하는 경우임
    // 반드시 완전히 다 보내기 전까지는 데이터를 보관하고, 다 보낸 다음 삭제해야함 <-- 정말 중요함
    return;
  }

  // 즉시 전송할 경우나, 큐 사이즈가 1개인 경우
  // HandleWrite 가 호출 (실제 전송 완료) 될때까지 버퍼 내용이 잘 보관 돼야 함
  // async_write_some 은 실제 전송이 다 안되어도 HandleWrite가 불릴 수 있다. (네트워크 환경이 좋지 않으면)
  // async_write 는 콜백 함수가 호출되었다면, 전송이 100프로 완료 된 것임.  
  boost::asio::async_write(
    socket_,
    boost::asio::buffer((msg.get())[0], (msg.get())->size()),
    [this](const boost::system::error_code& error, size_t bytes_transferred) // 람다식으로 변경
  {
    std::cout << "error " << error << std::endl;
    std::cout << "write bytes " << bytes_transferred << std::endl;
    // 보낸 데이터를 제거한다. (보통의 경우에는 이 부분에서 다 끝남)
    {
      auto will_delete = std::move(send_msg_queue_.front());
    }
    send_msg_queue_.pop_front();

    if (send_msg_queue_.empty() == false)
    {
      // 큐에 쌓인게 있을 경우 (이전에 전송하지 못한게 남아 있는 경우임)
      // 즉시 전송
      PostWrite(true, std::move(send_msg_queue_.front()));
    }
  }
  );
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

    server_->CloseSession(session_id_); // 에러가 있으면 연결 종료
  }
  else
  {
    // 받은 데이터를 패킷 버퍼에 저장
    memcpy(&packet_buffer_[packet_buffer_mark_], recv_buffer_.data(), byte_transferred);

    int recv_packet_data_size = packet_buffer_mark_ + byte_transferred;
    int read_data = 0;

    while (recv_packet_data_size > 0) { // 받은 데이터를 모두 처리할 때까지 반복
      if (recv_packet_data_size < PACKET_HEADER_SIZE)
      {
        // 남은 데이터가 패킷 헤더보다 작으면 중단
        break;
      }
      //int body_size = *((int*)packet_buffer_[read_data]); // TODO static_cast?
      int body_size = read32_be(&packet_buffer_[read_data]);
      int total_size = (PACKET_HEADER_SIZE + body_size);
      if (total_size <= recv_packet_data_size) {
        // 처리할 수 있는 만큼 데이터가 있다면 패킷 처리
        server_->ProcessPacket(session_id_, &packet_buffer_[read_data+ PACKET_HEADER_SIZE], body_size);
        recv_packet_data_size -= total_size;
        read_data += total_size;
      }
      else
      {
        // 패킷으로 처리할 수 있는 만큼이 아니면 중단
        break;
      }
    }

    if (recv_packet_data_size > 0) { // 남은 데이터는 패킷 버퍼에 저장
      char temp_buffer[MAX_RECEIVE_BUFFER_LEN] = { 0, };
      memcpy(&temp_buffer[0], &packet_buffer_[read_data], recv_packet_data_size);
      memcpy(&packet_buffer_[0], &temp_buffer[0], recv_packet_data_size); // 버퍼 제일 앞쪽으로 당김
    }

    // 남은 데이터 양을 저장하고 데이터 받기 요청
    packet_buffer_mark_ = recv_packet_data_size;
    PostRead();
  }
}