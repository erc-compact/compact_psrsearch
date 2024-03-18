#include "utils/gen_utils.hpp"
#include <memory>
#include <cstring> 

namespace IO {

    /**
     * @brief Base class for data buffers.
     * 
     * This class provides a base implementation for data buffers. It defines the common
     * properties and methods that all data buffers should have.
     */
    class DataBufferBase
    {
    protected:
        std::size_t startByte; /**< The starting byte index of the buffer. */
        std::size_t nBytes; /**< The number of bytes in the buffer. */

    public:
        /**
         * @brief Constructs a DataBufferBase object.
         * 
         * @param startByte The starting byte index of the buffer.
         * @param nBytes The number of bytes in the buffer.
         */
        DataBufferBase(std::size_t startByte, std::size_t nBytes)
            : startByte(startByte), nBytes(nBytes)
        {
        }

        /**
         * @brief Retrieves the buffer.
         * 
         * This method should be implemented by derived classes to provide access to the buffer.
         * 
         * @return A pointer to the buffer.
         */
        virtual void* getBuffer() = 0;

        /**
         * @brief Destroys the DataBufferBase object.
         * 
         * This is a virtual destructor to ensure proper destruction of derived classes.
         */
        virtual ~DataBufferBase() = default;
    };

    template <class DTYPE>
    class DataBuffer : public DataBufferBase
    {
        public:
            std::shared_ptr<DTYPE> buffer = nullptr; /**< The buffer storing the data. */

            /**
             * @brief Constructs a DataBuffer object.
             * 
             * @param startByte The starting byte index of the buffer.
             * @param nBytes The number of bytes in the buffer.
             */
            DataBuffer(std::size_t startByte, std::size_t nBytes)
                : DataBufferBase(startByte, nBytes)
            {
                buffer = std::make_shared<DTYPE[]>(nBytes);
            }

            /**
             * @brief Destroys the DataBuffer object.
             */
            ~DataBuffer() {}

            /**
             * @brief Retrieves the buffer.
             * 
             * @return A pointer to the buffer.
             */
            void* getBuffer() override {
                return static_cast<void*>(buffer.get());
            }

            /**
             * @brief Overloads the [] operator to access elements in the buffer.
             * 
             * @param idx The index of the element to access.
             * @return A reference to the element at the specified index.
             */
            DTYPE& operator[](std::size_t idx) {
                return buffer[idx];
            }

            /**
             * @brief Clears the buffer by setting all elements to zero.
             */
            void clearBuffer() {
                std::memset(buffer.get(), 0, nBytes);
            }
    };
};