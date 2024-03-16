#include "data/header_params.hpp"


 template <> int IO::HeaderParam<char *>::getValueLength() {
            std::string str_value = std::string(this->value);
            return str_value.length();
        }

        // can also use constexpr(std::is_arithmetic_v<t>)