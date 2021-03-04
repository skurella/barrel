#include <CL/sycl.hpp>
#include <CL/sycl/INTEL/fpga_extensions.hpp>

#include "dpc_common.hpp"

#include <iostream>
#include <vector>
#include <thread>
#include <variant>

// When a `Stop` packet arrives at a kernel, it stops.
struct Stop {};

struct Data {
    int payload;
};

using rx_packet_t = std::variant<Stop, Data>;
using tx_packet_t = std::variant<Stop, Data>;

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

// From <https://en.cppreference.com/w/cpp/utility/variant/visit>
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

class Receiver;
void receiver(sycl::queue& q) {
    q.submit([&](sycl::handler& h) {
        h.single_task<Receiver>([=]() {
            bool running = true;
            while (running) {
                auto packet = pipes::recv::read();
                if(std::holds_alternative<Stop>(packet)) {
                    running = false;
                    // Propagate the stop signal to transmitter
                    pipes::send::write(tx_packet_t(Stop{}));
                } else if(auto data = std::get_if<Data>(&packet)) {
                    // do something with the packet
                    pipes::send::write(tx_packet_t(Data { .payload = data->payload + 100 }));
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
    receive(q, rx_packet_t(Stop{}));
}

int main() {
    sycl::INTEL::fpga_emulator_selector device_selector;
    sycl::queue q { device_selector, dpc_common::exception_handler };

    receiver(q);

    std::thread transmitter_thread([&]() {
        bool running = true;
        while(running) {
            tx_packet_t packet = get_packet_to_transmit(q);
            std::visit(overloaded {
                [&](Stop) {
                    running = false;
                },
                [&](Data &data) {
                    std::cout << "Sending: " << data.payload << std::endl;
                },
            }, packet);
        }
    });

    for(int i = 0; i < 10; i++) {
        std::cout << "Receiving: " << i << std::endl;
        receive(q, rx_packet_t(Data { .payload = i }));
    }

    std::cout << "Stopping" << std::endl;
    stop(q);

    transmitter_thread.join();
}
