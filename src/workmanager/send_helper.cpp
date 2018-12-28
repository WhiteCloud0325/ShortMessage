#include "workmanager/send_helper.h"
#include "tools/logger.h"
#include <boost/shared_ptr.hpp>
#include <thrift/Thrift.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

using namespace apache;
using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;

SendHelper* SendHelper::instance_ = NULL;

bool SendHelper::Init(libconfig::Setting &setting) {
    try {
        int num = setting.getLength();
        if (num < 0) {
            return false;
        }
        for (int i = 0; i < num; ++i) {
            const libconfig::Setting &addr = setting[i];
            Address address;
            int id = 0;
            if (!addr.lookupValue("beamid", id)) {
                return false;
            }
            if (!addr.lookupValue("port", address.port)) {
                return false;
            }
            if (!addr.lookupValue("ip", address.ip)) {
                return false;
            }
            schedule_address_.insert(std::make_pair(id, address));
        }
        
    }
    catch (...) {
        LOG_INFO("SendHelper Init Exception");
        return false;
    }
    return true;
}

bool SendHelper::SendMessage(const uint32_t user_id, const std::string &buf, std::vector<SateParam> &sates, const int &level) {
    float cur_snr = -1;
    int32_t cur_sate_id = -1;
    int32_t cur_beam_id = -1;
    for (SateParam &sate: sates) {
        if (sate.snr > cur_snr && schedule_address_.find(sate.beam_id) != schedule_address_.end()) {
            cur_snr = sate.snr;
            cur_sate_id = sate.sate_id;
            cur_beam_id = sate.beam_id;
        }
    }
    if (cur_sate_id == -1) {
        return false;
    }
    try {
        im::AccessMessage send;
        send.__set_uid(user_id);
        send.__set_sate_id(cur_sate_id);
        send.__set_beam_id(cur_beam_id);
        send.__set_level(level);
        send.__set_content(buf);
        boost::shared_ptr<TSocket> client_socket(new TSocket(schedule_address_[cur_beam_id].ip, schedule_address_[cur_beam_id].port));
        boost::shared_ptr<TTransport> client_transport(new TBufferedTransport(client_socket));
        boost::shared_ptr<TProtocol> client_protocol(new TBinaryProtocol(client_transport));
        boost::shared_ptr<im::LogicInterfaceClient> client(new im::LogicInterfaceClient(client_protocol));
    
        client_transport->open();
        client->LogicToAccess(send);
        client_transport->close();
    }
    catch(...) {
        LOG_ERROR("SendMessage Error: user_id=%u||content=%s", user_id, buf.c_str());
        return false;
    }
    return true;
}
