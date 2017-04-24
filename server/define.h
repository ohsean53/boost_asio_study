#pragma once

const unsigned short PORT_NUMBER = 31400;
const int MAX_RECEIVE_BUFFER_LEN = 512;
const int MAX_NAME_LEN = 17;
const int MAX_MESSAGE_LEN = 129; // ? why?



struct PACKET_HEADER
{
  short id;
  short size;
};



//  ÆÐÅ¶
const short REQ_IN = 1;
// PKT_REQ_IN

const short RES_IN = 2;
// PKT_RES_IN

const short REQ_CHAT = 6;
// PKT_REQ_CHAT

const short NOTICE_CHAT = 7;
// PKT_NOTICE_CHAT


struct PKT_REQ_IN : public PACKET_HEADER
{
  void Init()
  {
    id = REQ_IN;
    size = sizeof(PKT_REQ_IN);
    memset(name, 0, MAX_NAME_LEN);
  }

  char name[MAX_NAME_LEN];
};

struct PKT_RES_IN : public PACKET_HEADER
{
  void Init()
  {
    id = RES_IN;
    size = sizeof(PKT_RES_IN);
    is_success = false;
  }

  bool is_success;
};

struct PKT_REQ_CHAT : public PACKET_HEADER
{
  void Init()
  {
    id = REQ_CHAT;
    size = sizeof(PKT_REQ_CHAT);
    memset(message, 0, MAX_MESSAGE_LEN);
  }

  char message[MAX_MESSAGE_LEN];
};

struct PKT_NOTICE_CHAT : public PACKET_HEADER
{
  void Init()
  {
    id = NOTICE_CHAT;
    size = sizeof(PKT_NOTICE_CHAT);
    memset(name, 0, MAX_NAME_LEN);
    memset(message, 0, MAX_MESSAGE_LEN);
  }

  char name[MAX_NAME_LEN];
  char message[MAX_MESSAGE_LEN];
};