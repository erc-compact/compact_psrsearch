#include "header_params.hpp"


 template <> int FileTypes::HeaderParam<char *>::get_value_length() {
            std::string str_value = std::string(this->value);
            return str_value.length();
        }

        // can also use constexpr(std::is_arithmetic_v<t>)