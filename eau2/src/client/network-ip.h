#pragma once
// lang:CwC

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "network.h"

/** Represents the information needed to identify a node. **/
class NodeInfo : public Object {
public:
  size_t id;
  sockaddr_in address;
};

/** IP based network. Each node is classified by an address and id. **/
class NetworkIP : public Network {
public:
};