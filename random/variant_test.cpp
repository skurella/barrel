#include <iostream>
#include <type_traits>
#include <variant>
#include <functional>

#include "myvariant.h"
 
struct Stop {
};
struct Data {
    int payload;
};

using packet_t = std::variant<Stop, Data>;

int main() {
    std::cout
      << std::boolalpha << "Stop is trivially copy-constructible? "
      << std::is_trivially_copy_constructible<Stop>::value
      << std::endl;

    std::cout
      << std::boolalpha << "Data is trivially copy-constructible? "
      << std::is_trivially_copy_constructible<Data>::value
      << std::endl;

    std::cout
      << std::boolalpha << "variant<int, bool> is trivially copy-constructible? "
      << std::is_trivially_copy_constructible<std::variant<int, bool>>::value
      << std::endl;

    std::cout
      << std::boolalpha << "packet_t is trivially copy-constructible? "
      << std::is_trivially_copy_constructible<packet_t>::value
      << std::endl;

    std::cout
      << std::boolalpha << "myvariant is trivially copy-constructible? "
      << std::is_trivially_copy_constructible<myvariant<Stop, Data>>::value
      << std::endl;
}
