#include "utils/gen_utils.hpp"
#include <memory>
#include <cstring> 

namespace IO {

    class DataBufferBase
    {

    protected:
        std::size_t startByte;
        std::size_t nBytes;

        DataBufferBase(std::size_t startByte, std::size_t nBytes)
            : startByte(startByte), nBytes(nBytes)
        {
        }
        virtual void* getBuffer() = 0;
        virtual ~DataBufferBase() = default;
    };

    template <class DTYPE>
    class DataBuffer : public DataBufferBase
    {
        public:
            std::shared_ptr<DTYPE[]> buffer;
            DataBuffer(std::size_t startByte, std::size_t nBytes)
                : DataBufferBase(startByte, nBytes)
            {
                buffer = std::shared_ptr<DTYPE[]>(safeNew1D<DTYPE>(nBytes, __func__));;
            }
            ~DataBuffer() {}

            void* getBuffer() override {
                return static_cast<void*>(buffer.get());
            }

            //overload the [] operator
            DTYPE& operator[](std::size_t idx) {
                return buffer[idx];
            }

            void clearBuffer() {
                std::memset(buffer.get(), 0, nBytes);
            }
            

            

    };

};