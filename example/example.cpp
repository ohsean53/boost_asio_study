// example.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include <iostream>
#include <memory>
#include <vector>
#include <array>
#include <ctime>
#include <chrono>
#include "../proto/protocol.pb.h"


using namespace std;
using ns = chrono::nanoseconds;
using get_time = chrono::steady_clock;
int main()
{
  std::cout << "test protobuf" << std::endl;

  // encoding
  gs_protocol::Message msg1;
  auto login_messasge = msg1.mutable_req_login();
  login_messasge->set_userid(123);
  auto msg_size = msg1.ByteSize();

  char* buffer = new char[msg_size];
  bool is_serialize_success;

  long long total = 0;
  //for (int i = 0; i < 1000; i++) {
  //  auto start = get_time::now();
  //  is_serialize_success = msg1.SerializeToArray(buffer, msg_size);
  //  auto end = get_time::now();
  //  auto diff = end - start;
  //  total += chrono::duration_cast<ns>(diff).count();
  //}
  //std::cout << "c array avg elapsed_time : " << total / 1000 << " ns " << std::endl;


  total = 0;
  auto vBuffer = std::make_shared<std::vector<char>>(msg_size);
  for (int i = 0; i < 1; i++) {
    auto start = get_time::now();
    is_serialize_success = msg1.SerializeToArray(&(vBuffer.get())[0], msg_size);
    auto end = get_time::now();
    auto diff = end - start;
    total += chrono::duration_cast<ns>(diff).count();
  }
  std::cout << "vector avg elapsed_time : " << total / 1000 << " ns " << std::endl;

  if (is_serialize_success) {
    std::cout << "vector serialize success!" << std::endl;
    std::cout << msg1.req_login().userid() << std::endl;
  }

  /* boost::asio::streambuf stream_buffer;
   std::ostream output_stream(&stream_buffer);
   total = 0;
   for (int i = 0; i < 1000; i++) {
     auto start = get_time::now();
     ::google::protobuf::io:: raw_output_stream(&output_stream);
     ::google::protobuf::io::CodedOutputStream coded_output_stream(&raw_output_stream);
     is_serialize_success = msg1.SerializeToArray(buffer, msg_size);
     std::vector<int> v(buffer, msg_size + buffer);
     auto end = get_time::now();
     auto diff = end - start;
     total += chrono::duration_cast<ns>(diff).count();
   }
   std::cout << "array to vector avg elapsed_time : " << total / 1000 << " ns " << std::endl;*/


   //total = 0;
   //std::string strbuffer;
   //for (int i = 0; i < 1000; i++) {
   //  auto start = get_time::now();
   //  is_serialize_success = msg1.SerializeToString(&strbuffer);
   //  auto end = get_time::now();
   //  auto diff = end - start;
   //  total += chrono::duration_cast<ns>(diff).count();
   //}
   //std::cout << "string avg elapsed_time : " << total / 1000 << " ns " << std::endl;


   //if (is_serialize_success) {
   //  std::cout << "serialize success!" << std::endl;
   //  std::cout << msg1.req_login().userid() << std::endl;
   //}

   // send to network

   // decoding
  gs_protocol::Message msg2;
  char testBuffer[4] = { 18,2,8,123 };
  //char testBuffer[4] = { 0, };

  //memcpy(testBuffer, static_cast<void*>(vBuffer.get()), msg_size);

  /*auto is_parse_success = msg2.ParsePartialFromArray(static_cast<void*>(vBuffer.get()), msg_size);*/
  auto is_parse_success = msg2.ParsePartialFromArray(testBuffer, msg_size);
  //auto is_parse_success = msg2.ParsePartialFromArray(buffer, msg_size);
  if (is_parse_success) {
    switch (msg2.payload_case())
    {
    case gs_protocol::Message::kReqLogin:
    {
      auto req_login = msg2.req_login();
      std::cout << "req_login parse success" << std::endl;
      std::cout << "user_id : " << req_login.userid() << std::endl;
    }
    break;

    case gs_protocol::Message::kReqCreate:
      break;
    case gs_protocol::Message::kReqJoin:
      break;
    case gs_protocol::Message::kReqAction1:
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
      break;
    case gs_protocol::Message::kNotifyQuit:
      break;
    default:
      std::cout << "not defined!" << std::endl;
      break;
    }
  }

  return 0;
}

