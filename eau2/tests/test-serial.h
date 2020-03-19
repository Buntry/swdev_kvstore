#pragma once

#include "../src/client/message.h"
#include "test-macros.h"
#include <gtest/gtest.h>

/**
 * @brief Unit tests for Serializable objects.
 *
 * @author griep.p@husky.neu.edu, colabella.a@husky.neu.edu
 */
class SerialTest : public ::testing::Test {
public:
  SerializableStringArray ssa;
  SerializableStringArray ssa_clone;
  SerializableDoubleArray sda;
  SerializableIntArray sia;
  SerializableIntArray sia_clone;
  size_t max_size = 1024;
  char *msg;
  char *buf;
  char *read;
  Ack a;
  Nack n;
  Put p;
  Reply r;
  Get g;
  WaitAndGet w;
  Status s;
  Kill k;
  Directory d;
  Register reg;

  void SetUp() {
    msg = new char[max_size];
    buf = msg;
    read = msg;

    ssa.push_back(new String("Apply"));
    ssa.push_back(new String("Banana"));
    ssa.push_back(nullptr);
    ssa.push_back(new String("Cucumber"));

    ssa_clone.push_back(new String("Apply"));
    ssa_clone.push_back(new String("Banana"));
    ssa_clone.push_back(nullptr);
    ssa_clone.push_back(new String("Cucumber"));

    sda.push_back(1.04);
    sda.push_back(-2.06);
    sda.push_back(19.0123124);
  }

  void TearDown() { delete[] msg; }
};

TEST_F(SerialTest, ArrayEncodeDecode) {
  sda.encode(buf);
  SerializableDoubleArray sda2;
  sda2.decode(read);
  ASSERT(sda.equals(&sda2));

  ssa.encode(buf);
  SerializableStringArray ssa2;
  ssa2.decode(read);
  ASSERT(ssa.equals(&ssa2));
}

TEST_F(SerialTest, AckMessage) {
  ASSERT(a.kind_ == MsgKind::Ack);
  a.init(0, 0, 0);
  a.encode(buf);
  Ack a2;
  a2.init(1, 1, 1);
  a2.decode(read);
  ASSERT(a2.sender_ == 0);
  ASSERT(a2.target_ == 0);
  ASSERT(a2.id_ == 0);
}

TEST_F(SerialTest, NackMessage) {
  ASSERT(n.kind_ == MsgKind::Nack);
  n.init(0, 0, 0);
  n.encode(buf);
  Nack n2;
  n2.init(1, 1, 1);
  n2.decode(read);
  ASSERT(n2.sender_ == 0);
  ASSERT(n2.target_ == 0);
  ASSERT(n2.id_ == 0);
}

TEST_F(SerialTest, PutMessage) {
  String *s1 = new String("hello");
  String *s2 = new String("hello");
  ASSERT(p.kind_ == MsgKind::Put);
  p.init(0, 0, 0);
  p.set(s1);
  p.encode(buf);
  Put p2;
  p2.decode(read);
  ASSERT(p2.target_ == 0);
  ASSERT(p2.sender_ == 0);
  ASSERT(p2.id_ == 0);
  ASSERT(p2.msg_->equals(s2));
}

TEST_F(SerialTest, ReplyMessage) {
  String *s1 = new String("hello");
  String *s2 = new String("hello");
  ASSERT(r.kind_ == MsgKind::Reply);
  r.init(0, 0, 0);
  r.set(s1);
  r.encode(buf);
  Reply r2;
  r2.decode(read);
  ASSERT(r.target_ == 0);
  ASSERT(r.sender_ == 0);
  ASSERT(r.id_ == 0);
  ASSERT(r.msg_->equals(s2));
}

TEST_F(SerialTest, GetMessage) {
  ASSERT(g.kind_ == MsgKind::Get);
  g.init(0, 0, 0);
  g.encode(buf);
  Get g2;
  g2.init(1, 1, 1);
  g2.decode(read);
  ASSERT(g.target_ == 0);
  ASSERT(g.sender_ == 0);
  ASSERT(g.id_ == 0);
}

TEST_F(SerialTest, WaitAndGetMessage) {
  ASSERT(w.kind_ == MsgKind::WaitAndGet);
  w.init(1, 1, 1);
  w.set(1);
  w.encode(buf);
  WaitAndGet w2;
  w2.decode(read);
  ASSERT(w2.sender_ == 1);
  ASSERT(w2.target_ == 1);
  ASSERT(w2.id_ == 1);
  ASSERT(w2.wait_ms_ == 1);
}

TEST_F(SerialTest, StatusMessage) {
  String *q1 = new String("hello");
  String *q2 = new String("hello");
  ASSERT(s.kind_ == MsgKind::Status);
  s.init(0, 0, 0);
  s.set(q1);
  s.encode(buf);
  Status s2;
  s2.decode(read);
  ASSERT(s2.sender_ == 0);
  ASSERT(s2.target_ == 0);
  ASSERT(s2.id_ == 0);
  ASSERT(s2.msg_->equals(q2));
}

TEST_F(SerialTest, KillMessage) {
  ASSERT(k.kind_ == MsgKind::Kill);
  k.init(0, 0, 0);
  k.encode(buf);
  Kill k2;
  k2.init(1, 1, 1);
  k2.decode(read);
  ASSERT(k2.id_ == 0);
  ASSERT(k2.sender_ == 0);
  ASSERT(k2.target_ == 0);
}

TEST_F(SerialTest, DirectoryMessage) {
  String *s1 = new String("hello");
  String *s2 = new String("hello");
  ASSERT(d.kind_ == MsgKind::Directory);
  d.init(0, 0, 0);
  d.add(s1, 8080);
  d.encode(buf);
  Directory d2;
  d2.decode(read);
  ASSERT(d2.sender_ == 0);
  ASSERT(d2.target_ == 0);
  ASSERT(d2.id_ == 0);
  ASSERT(d2.clients = 1);
  ASSERT(d2.addresses.get(0)->equals(s2));
}

TEST_F(SerialTest, RegisterMessage) {
  sockaddr_in q;
  q.sin_family = AF_INET;

  ASSERT(reg.kind_ == MsgKind::Register);
  reg.init(0, 0, 0);
  reg.set(q, 1);
  reg.encode(buf);
  Register r2;
  r2.decode(read);
  ASSERT(r2.sender_ == 0);
  ASSERT(r2.target_ == 0);
  ASSERT(r2.id_ == 0);
  ASSERT(r2.port == 1);
  ASSERT(r2.client.sin_family == AF_INET);
}
