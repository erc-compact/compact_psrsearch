#include "search_mode_file.hpp"
#include "header_params.hpp"
#include "constants.hpp"
#include "gen_utils.hpp"
#include "sigproc_utils.hpp"
#include <string>
#include <cstring>

using namespace FileTypes;
/**
 * @brief Retrieves the header parameter with the specified key.
 *
 * @param key The key of the header parameter to retrieve.
 * @return A pointer to the header parameter if found, otherwise NULL.
 */
HeaderParamBase *SearchModeFile::get_header_param(std::string key)
{
    for (std::vector<HeaderParamBase *>::iterator it = header_params.begin(); it != header_params.end(); ++it)
    {
        if ((*it)->key == key)
        {
            return *it;
        }
    }
    return NULL;
}

int SearchModeFile::remove_header_param(std::string key)
{
    for (std::vector<HeaderParamBase *>::iterator it = header_params.begin(); it != header_params.end(); ++it)
    {
        if (!std::strcmp((*it)->key.c_str(), key.c_str()))
        {
            header_params.erase(it);
            return EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}

void SearchModeFile::pretty_print_header()
{
    int max_key_length = 3, max_value_length = 5;

    // Find the maximum length of keys and values
    for (const auto &header_param : header_params)
    {
        max_key_length = std::max(max_key_length, header_param->get_key_length());
        max_value_length = std::max(max_value_length, header_param->get_value_length());
    }
    max_key_length += 2;
    max_value_length += 2;

    std::cout << std::left << std::setw(max_key_length) << "Key" << std::setw(max_value_length) << "Value"
              << "\n";
    std::cout << std::string(max_value_length + max_key_length, '-') << "\n";

    for (const auto &header_param : header_params)
    {
        if (header_param != nullptr)
        {
            header_param->pretty_print(max_key_length, max_value_length);
        }
    }
}

void SearchModeFile::print_header()
{
    for (std::vector<FileTypes::HeaderParamBase *>::iterator it = header_params.begin(); it != header_params.end(); ++it)
    {
        HeaderParamBase *base = *it;
        if (!base->inheader)
            continue;
        std::cerr << base->key << "(" << base->dtype << "): ";
        if (base->dtype == std::string(INT))
        {
            std::cerr << (static_cast<HeaderParam<int> *>(base))->value << std::endl;
        }
        else if (base->dtype == std::string(LONG))
        {
            std::cerr << (static_cast<HeaderParam<long> *>(base))->value << std::endl;
        }
        else if (base->dtype == std::string(DOUBLE))
        {
            double value = (static_cast<HeaderParam<double> *>(base))->value;
            if (base->key == std::string(SRC_RAJ))
            {
                std::string str_value;
                sigproc_to_hhmmss(value, str_value);
                std::cerr << str_value << std::endl;
            }
            else if (base->key == std::string(SRC_DEJ))
            {
                std::string str_value;
                sigproc_to_ddmmss(value, str_value);
                std::cerr << str_value << std::endl;
            }
            else
            {
                std::cerr << value << std::endl;
            }
        }
        else if (base->dtype == std::string(STRING))
        {
            if ((static_cast<HeaderParam<char *> *>(base))->value != NULL)
                std::cerr << (static_cast<HeaderParam<char *> *>(base))->value << std::endl;
        }
        else if (base->dtype == std::string(NULL_STR))
        {
            std::cerr << "NULL" << std::endl;
        }
    }
}
