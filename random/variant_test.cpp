#include <iostream>
#include <type_traits>
#include <variant>
#include <functional>

#include "../utils/variant.h"

using utils::variant;
 
struct Stop {
};
struct Data {
    int payload;
};

using packet_t = variant<Stop, Data>;

int main() {
    std::cout
      << std::boolalpha << "packet_t is trivially copy-constructible? "
      << std::is_trivially_copy_constructible<packet_t>::value
      << std::endl;

//    packet_t p{Stop{}};
//    packet_t p1;
}
