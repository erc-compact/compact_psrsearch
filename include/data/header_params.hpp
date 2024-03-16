#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
/**
 * @namespace IO
 * @brief Contains classes related to file types and header parameters.
 */
namespace IO {
    /**
     * @brief Base class for header parameters.
     */
    class HeaderParamBase // needed to make a list of different valued header parameters
    {
    public:
        bool inheader; /**< Flag indicating if the parameter is present in the header of the data. */
        std::string dtype; /**< Data type of the parameter. Specified in header_keys.dat */
        std::string key; /**< Key associated with the parameter. */

        /**
         * @brief Overloaded equality operator.
         * @param d The HeaderParamBase object to compare with.
         * @return True if the objects are equal, false otherwise.
         */
        
        bool operator==(const HeaderParamBase &d) const 
        {
            std::cerr << this->key << " " << d.key << std::endl;
            return (this->key == d.key);
        }



        /**
         * @brief Prints the object in a pretty format.
         * 
         * @param keyWidth The width of the key column in the output.
         * @param valueWidth The width of the value column in the output.
         */
        virtual void prettyPrint(int keyWidth, int valueWidth) = 0;

        /**
         * @brief Gets the length of the key.
         * @return The length of the key.
         */
        virtual int getKeyLength() = 0;

        /**
         * @brief Gets the length of the value.
         * @return The length of the value.
         */
        virtual int getValueLength() = 0;

        /**
         * @brief Prints the header parameters.
         */
        virtual void print() = 0;

        /**
         * @brief Destructor for the HeaderParamBase class.
         */
        virtual ~HeaderParamBase() {}

    };
    
    /**
     * @brief Represents a header parameter.
     * 
     * This class is derived from HeaderParamBase and provides functionality to store and manipulate header parameters.
     * It contains member variables for key, data type, value, and a flag indicating whether the parameter is present in the header.
     * The class provides constructors to initialize the key, data type, and value of the parameter.
     * It also provides a constructor to create a HeaderParam object from a HeaderParamBase object.
     * The class includes methods to pretty print the parameter, get the length of the key, and get the length of the value.
     */
    template <class T>
    class HeaderParam : public HeaderParamBase
    {

    public:
        T value;

        /**
         * @brief Represents a header parameter.
         * 
         * This class encapsulates a header parameter, which consists of a key and a data type.
         */
        inline HeaderParam(std::string key, std::string dtype) {
            this->key = key;
            this->dtype = dtype;
            this->inheader = false;
        }

        /**
         * @brief Constructs a HeaderParam object with the specified key, data type, and value.
         * 
         * @param key The key associated with the header parameter.
         * @param dtype The data type of the header parameter.
         * @param value The value of the header parameter.
         */
        inline HeaderParam(std::string key, std::string dtype, T value) {
            this->key = key;
            this->dtype = dtype;
            this->value = value;
            this->inheader = true;
        }

        /**
         * @brief Constructor for HeaderParam class.
         * @param param_base Pointer to the base class object.
         */
        inline HeaderParam(HeaderParamBase *param_base) {

            HeaderParam<T> *param = dynamic_cast<HeaderParam<T> *>(param_base);
            if (param == NULL) {
                std::cerr << "param is null. Possibly an incorrect dynamic cast??" << std::endl;
                return;
            }

            this->key = param->key;
            this->dtype = param->dtype;
            this->value = param->value;
            this->inheader = param->inheader;
        }

        /**
         * @brief Destructor for the HeaderParam class.
         */
        virtual ~HeaderParam() {}

        /**
         * @brief Prints the object in a pretty format.
         * 
         * @param keyWidth The width of the key column in the output.
         * @param valueWidth The width of the value column in the output.
         */
        void prettyPrint(int keyWidth, int valueWidth) {
            std::cout << std::left << std::setw(keyWidth) << this->key << std::setw(valueWidth) << this->value << "\n";
        }

        /**
         * @brief Prints the header parameters.
         */
        void print(){
            std::cout << this->key << " " << this->value << std::endl;
        }

        /**
         * @brief Get the length of the key.
         * 
         * @return int The length of the key.
         */
        int getKeyLength() {
            return this->key.length();
        }
        /**
         * @brief Get the length of the value.
         * 
         * @return int The length of the value.
         */
        int getValueLength() {
            return std::to_string(this->value).length();
        }
        

    };
    template <> 
    int HeaderParam<char*>::getValueLength();

    
    
};
