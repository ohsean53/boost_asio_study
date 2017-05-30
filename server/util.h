#pragma once

#include "stdafx.h"
#include "chat_server.h"
#include "../proto/protocol.pb.h"

std::shared_ptr<std::vector<char>> SerializePBMessage(const gs_protocol::Message& msg);
int32_t read32_be(const char* b);
void write32_be(char *b, int32_t n);