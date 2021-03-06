/*
 * Copyright (c) 2014 Zubax, zubax.com
 * Distributed under the MIT License, available in the file LICENSE.
 * Author: Pavel Kirienko <pavel.kirienko@zubax.com>
 */

#include <iostream>
#include <cassert>
#include <stdexcept>
#include <uavcan_linux/uavcan_linux.hpp>
#include <uavcan/equipment/air_data/StaticPressure.hpp>
#include <uavcan/equipment/air_data/StaticTemperature.hpp>
#include <uavcan/equipment/ahrs/Magnetometer.hpp>
#include <uavcan/equipment/gnss/Fix.hpp>
#include <uavcan/equipment/gnss/Auxiliary.hpp>

#ifndef STRINGIZE
#  define STRINGIZE2(x)   #x
#  define STRINGIZE(x)    STRINGIZE2(x)
#endif
#define ENFORCE(x) if (!(x)) { throw std::runtime_error(__FILE__ ":" STRINGIZE(__LINE__) ": " #x); }

static uavcan_linux::NodePtr initNode(const std::vector<std::string>& ifaces, uavcan::NodeID nid,
                                      const std::string& name)
{
    auto node = uavcan_linux::makeNode(ifaces);
    node->setNodeID(nid);
    node->setName(name.c_str());

    ENFORCE(0 <= node->start());

    node->setModeOperational();
    return node;
}

template <typename DataType>
static void printMessage(const uavcan::ReceivedDataStructure<DataType>& msg)
{
    std::cout << "[" << DataType::getDataTypeFullName() << "]\n" << msg << "\n---" << std::endl;
}

template <typename DataType>
static std::shared_ptr<uavcan::Subscriber<DataType>>
makePrintingSubscriber(const uavcan_linux::NodePtr& node)
{
    return node->makeSubscriber<DataType>(&printMessage<DataType>);
}

static void runForever(const uavcan_linux::NodePtr& node)
{
    auto sub_log      = makePrintingSubscriber<uavcan::protocol::debug::LogMessage>(node);
    auto sub_pressure = makePrintingSubscriber<uavcan::equipment::air_data::StaticPressure>(node);
    auto sub_temperat = makePrintingSubscriber<uavcan::equipment::air_data::StaticTemperature>(node);
    auto sub_mag      = makePrintingSubscriber<uavcan::equipment::ahrs::Magnetometer>(node);
    auto sub_gnss_fix = makePrintingSubscriber<uavcan::equipment::gnss::Fix>(node);
    auto sub_gnss_aux = makePrintingSubscriber<uavcan::equipment::gnss::Auxiliary>(node);

    while (true)
    {
        const int res = node->spin(uavcan::MonotonicDuration::getInfinite());
        if (res < 0)
        {
            node->logError("spin", "Error %*", res);
        }
    }
}

int main(int argc, const char** argv)
{
    if (argc < 3)
    {
        std::cout << "Usage:\n\t" << argv[0] << " <node-id> <can-iface-name-1> [can-iface-name-N...]" << std::endl;
        return 1;
    }
    const int self_node_id = std::stoi(argv[1]);
    std::vector<std::string> iface_names;
    for (int i = 2; i < argc; i++)
    {
        iface_names.emplace_back(argv[i]);
    }
    uavcan_linux::NodePtr node = initNode(iface_names, self_node_id, "com.zubax.gnss.test");
    runForever(node);
    return 0;
}
