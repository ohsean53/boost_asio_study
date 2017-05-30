#pragma once
#include "stdafx.h"
#include "define.h"
#include "../proto/protocol.pb.h"

class ChatClient
{
public:
  ChatClient(boost::asio::io_service& io_service);
  virtual ~ChatClient();

  bool IsConnecting();
  bool IsLoing();
  void Login();
  void Connect(boost::asio::ip::tcp::endpoint endpoint);
  void Close();
  void PostSend(const bool immediately, const int size, std::unique_ptr<std::array<char, MAX_RECEIVE_BUFFER_LEN>> data);

private :
  void HandleConnect(const boost::system::error_code& error);
  void HandleWrite(const boost::system::error_code& error, size_t bytes_transferred);
  void HandleRead(const boost::system::error_code& error, size_t bytes_transferred);
  void ProcessPacket(const char* data);

  boost::asio::io_service& io_service_;
  boost::asio::ip::tcp::socket socket_;

  std::array<char, MAX_RECEIVE_BUFFER_LEN> recv_buffer_;

  int packet_buffer_mark_;
  std::array<char, MAX_RECEIVE_BUFFER_LEN * 2> packet_buffer_;

  // for thread race condition
  std::mutex mutex_;
  std::condition_variable cond_var_;

  //std::deque< char* > send_data_queue_;
  std::deque<std::unique_ptr<std::array<char, MAX_RECEIVE_BUFFER_LEN>>> send_data_queue_;

  bool is_login_;
};
