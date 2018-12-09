/**
 * Autogenerated by Thrift Compiler (0.9.3)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "im_types.h"

#include <algorithm>
#include <ostream>

#include <thrift/TToString.h>

namespace im {


Request::~Request() throw() {
}


void Request::__set_type(const int32_t val) {
  this->type = val;
}

void Request::__set_content(const std::string& val) {
  this->content = val;
}

uint32_t Request::read(::apache::thrift::protocol::TProtocol* iprot) {

  apache::thrift::protocol::TInputRecursionTracker tracker(*iprot);
  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_I32) {
          xfer += iprot->readI32(this->type);
          this->__isset.type = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->content);
          this->__isset.content = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t Request::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  apache::thrift::protocol::TOutputRecursionTracker tracker(*oprot);
  xfer += oprot->writeStructBegin("Request");

  xfer += oprot->writeFieldBegin("type", ::apache::thrift::protocol::T_I32, 1);
  xfer += oprot->writeI32(this->type);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("content", ::apache::thrift::protocol::T_STRING, 2);
  xfer += oprot->writeString(this->content);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(Request &a, Request &b) {
  using ::std::swap;
  swap(a.type, b.type);
  swap(a.content, b.content);
  swap(a.__isset, b.__isset);
}

Request::Request(const Request& other0) {
  type = other0.type;
  content = other0.content;
  __isset = other0.__isset;
}
Request& Request::operator=(const Request& other1) {
  type = other1.type;
  content = other1.content;
  __isset = other1.__isset;
  return *this;
}
void Request::printTo(std::ostream& out) const {
  using ::apache::thrift::to_string;
  out << "Request(";
  out << "type=" << to_string(type);
  out << ", " << "content=" << to_string(content);
  out << ")";
}


Response::~Response() throw() {
}


void Response::__set_type(const int32_t val) {
  this->type = val;
}

void Response::__set_uid(const int64_t val) {
  this->uid = val;
}

void Response::__set_content(const std::string& val) {
  this->content = val;
}

void Response::__set_ip(const std::string& val) {
  this->ip = val;
}

uint32_t Response::read(::apache::thrift::protocol::TProtocol* iprot) {

  apache::thrift::protocol::TInputRecursionTracker tracker(*iprot);
  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_I32) {
          xfer += iprot->readI32(this->type);
          this->__isset.type = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_I64) {
          xfer += iprot->readI64(this->uid);
          this->__isset.uid = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->content);
          this->__isset.content = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 4:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->ip);
          this->__isset.ip = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t Response::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  apache::thrift::protocol::TOutputRecursionTracker tracker(*oprot);
  xfer += oprot->writeStructBegin("Response");

  xfer += oprot->writeFieldBegin("type", ::apache::thrift::protocol::T_I32, 1);
  xfer += oprot->writeI32(this->type);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("uid", ::apache::thrift::protocol::T_I64, 2);
  xfer += oprot->writeI64(this->uid);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("content", ::apache::thrift::protocol::T_STRING, 3);
  xfer += oprot->writeString(this->content);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("ip", ::apache::thrift::protocol::T_STRING, 4);
  xfer += oprot->writeString(this->ip);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(Response &a, Response &b) {
  using ::std::swap;
  swap(a.type, b.type);
  swap(a.uid, b.uid);
  swap(a.content, b.content);
  swap(a.ip, b.ip);
  swap(a.__isset, b.__isset);
}

Response::Response(const Response& other2) {
  type = other2.type;
  uid = other2.uid;
  content = other2.content;
  ip = other2.ip;
  __isset = other2.__isset;
}
Response& Response::operator=(const Response& other3) {
  type = other3.type;
  uid = other3.uid;
  content = other3.content;
  ip = other3.ip;
  __isset = other3.__isset;
  return *this;
}
void Response::printTo(std::ostream& out) const {
  using ::apache::thrift::to_string;
  out << "Response(";
  out << "type=" << to_string(type);
  out << ", " << "uid=" << to_string(uid);
  out << ", " << "content=" << to_string(content);
  out << ", " << "ip=" << to_string(ip);
  out << ")";
}

} // namespace