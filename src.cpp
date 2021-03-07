#include <CL/sycl.hpp>
#include <CL/sycl/INTEL/fpga_extensions.hpp>

#include "dpc_common.hpp"

#include "utils/variant.h"

#include <iostream>
#include <vector>
#include <thread>

using utils::variant;

enum packet_type_t {
    // When a `Stop` packet arrives at a kernel, it stops.
    Stop,

    Data,
};

struct packet_t {
    packet_type_t type;
    int payload;
};

using rx_packet_t = packet_t;
using tx_packet_t = packet_t;

namespace pipes {
    using recv = sycl::pipe<class p_recv, rx_packet_t>;
    using send = sycl::pipe<class p_send, tx_packet_t>;
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
            bool running = true;
            while (running) {
                auto packet = pipes::recv::read();
                switch(packet.type) {
                case Stop:
                    running = false;
                    // Propagate the stop signal to transmitter
                    pipes::send::write(packet_t { .type = Stop });
                    break;
                case Data:
                    // do something with the packet
                    pipes::send::write(packet_t { .type = Data, .payload = packet.payload + 100 });
                    break;
                }
            }
        });
    });
}

class TransmitterGateway;
tx_packet_t get_packet_to_transmit(sycl::queue& q) {
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

void stop(sycl::queue& q) {
    receive(q, packet_t { .type = Stop });
}

void whatis(variant<int, bool> v) {
    v.visit<void>(
        [](int i) { std::cout << "int" << i << std::endl; },
        [](bool b) { std::cout << "bool" << b << std::endl; }
    );
}

int main() {
    sycl::INTEL::fpga_emulator_selector device_selector;
    sycl::queue q { device_selector, dpc_common::exception_handler };

    receiver(q);

    std::thread transmitter_thread([&]() {
        bool running = true;
        while(running) {
            tx_packet_t packet = get_packet_to_transmit(q);
            switch(packet.type) {
            case Stop:
                running = false;
                break;
            case Data:
                std::cout << "Sending: " << packet.payload << std::endl;
                break;
            }
        }
    });

    for(int i = 0; i < 10; i++) {
        std::cout << "Receiving: " << i << std::endl;
        receive(q, packet_t { .type = Data, .payload = i });
    }

    std::cout << "Stopping" << std::endl;
    stop(q);

    transmitter_thread.join();

    variant<int, bool> v_int{1};
    variant<int, bool> v_bool{false};

    whatis(v_int);
    whatis(v_bool);
}
