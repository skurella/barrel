#include <CL/sycl.hpp>
#include <CL/sycl/INTEL/fpga_extensions.hpp>

#include "dpc_common.hpp"

#include <iostream>
#include <vector>

using rx_packet_t = int;
using tx_packet_t = int;

namespace pipes {
    using recv = sycl::pipe<class p_recv, rx_packet_t>;
    using send = sycl::pipe<class p_send, rx_packet_t>;
}

class ReceiverGateway;
void receive(sycl::queue& q, rx_packet_t packet) {
    q.submit([&](sycl::handler& h) {
        h.single_task<ReceiverGateway>([=]() {
            pipes::recv::write(packet);
        });
    });
}

class Receiver;
void receiver(sycl::queue& q) {
    q.submit([&](sycl::handler& h) {
        h.single_task<Receiver>([=]() {
            while (true) {
                auto packet = pipes::recv::read();
                // do something with the packet
                pipes::send::write(packet + 100);
            }
        });
    });
}

class TransmitterGateway;
tx_packet_t send(sycl::queue& q) {
    std::vector<tx_packet_t> data(1);
    sycl::buffer buf_data { data };
    sycl::event e = q.submit([&](sycl::handler& h) {
        sycl::accessor acc_data { buf_data, h, sycl::write_only };
        h.single_task<TransmitterGateway>([=]() {
            acc_data[0] = pipes::send::read();
        });
    });
    e.wait();
    return data[0];
}

int main() {
    sycl::INTEL::fpga_emulator_selector device_selector;
    sycl::queue q { device_selector, dpc_common::exception_handler };

    receiver(q);

    receive(q, 50);

    std::cout << send(q) << std::endl;
}
