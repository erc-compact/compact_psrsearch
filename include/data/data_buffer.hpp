#include "utils/gen_utils.hpp"
#include <memory>
namespace IO {

    class DataBufferBase
    {

    public:
        std::size_t start_byte;
        std::size_t nbytes;

        DataBufferBase(std::size_t start_byte, std::size_t nbytes)
            : start_byte(start_byte), nbytes(nbytes)
        {
        }
        virtual ~DataBufferBase() = default;

    };

    template <class DTYPE>
    class DataBuffer : public DataBufferBase
    {
        public:
            std::unique_ptr<DTYPE[]> buffer;
            DataBuffer(std::size_t start_byte, std::size_t nbytes)
                : DataBufferBase(start_byte, nbytes)
            {
                buffer = std::unique_ptr<DTYPE[]>(safe_new_1D<DTYPE>(nbytes, __func__));;
            }
            ~DataBuffer() {}

            //overload the [] operator to index the buffer when called data_buffer[i]
            DTYPE &operator[](std::size_t i)
            {
                return buffer[i];
            }

            

    };

};