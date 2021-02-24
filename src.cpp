#include <CL/sycl.hpp>
#include <CL/sycl/INTEL/fpga_extensions.hpp>

#include "dpc_common.hpp"

#include <iostream>
#include <vector>

class Kernel;

int main() {
    sycl::INTEL::fpga_emulator_selector device_selector;
    sycl::queue q { device_selector, dpc_common::exception_handler };

    std::vector<int> data { 10 };
    sycl::buffer buf_data { data };

    q.submit([&](sycl::handler& h) {
        // sycl::accessor acc_data { buf_data, h, sycl::read_write };
        sycl::accessor acc_data(buf_data, h, sycl::read_write);

        h.single_task<Kernel>([=]() {
             acc_data[0] = 20;
        });
    });
    q.wait();

    std::cout << data[0] << std::endl;
}
