#pragma once
#include "utils/gen_utils.hpp"
#include <memory>
#include <cstring> 
#include <vector>

namespace IO {
    class DataBufferBase;
    template <class DTYPE> class DataBuffer;

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

        // getters for startByte and nBytes
        std::size_t getStartByte() { return startByte; }
        std::size_t getNBytes() { return nBytes; }



        /**
         * @brief Retrieves the number of elements in the buffer.
         * 
         * This method should be implemented by derived classes to provide the number of elements in the buffer.
         * 
         * @return The number of elements in the buffer.
         */
        virtual int getNElements() = 0;



        /**
         * @brief Destroys the DataBufferBase object.
         * 
         * This is a virtual destructor to ensure proper destruction of derived classes.
         */
        virtual ~DataBufferBase() = default;

        virtual void clearBuffer() = 0;

        virtual double getDoubleValueAt(std::size_t idx) = 0;   


        template <typename DTYPE>
        void loadData(std::size_t startByte, std::shared_ptr<std::vector<DTYPE>> dataBuffer) {
            DataBuffer<DTYPE>* derived = dynamic_cast<DataBuffer<DTYPE>*>(this);

            if (derived) {
                derived->buffer = dataBuffer;
                derived->nBytes = dataBuffer->size() * sizeof(DTYPE);
                derived->nElements = dataBuffer->size();
                derived-> startByte = startByte;
            } else {
                throw std::bad_cast(); 
            }
        }

        template <typename DTYPE>
        std::shared_ptr<std::vector<DTYPE>> getBuffer() {
            const DataBuffer<DTYPE>* derived = dynamic_cast<const DataBuffer<DTYPE>*>(this);
            if (derived) {
                return derived->getTypedBuffer();
            } else {
                throw std::bad_cast(); 
            }
        }
    };

    template <class DTYPE>
    class DataBuffer : public DataBufferBase
    {
        public:
            std::shared_ptr<std::vector<DTYPE>> buffer = nullptr; /**< The buffer storing the data. */
            std::size_t nElements; /**< The number of elements in the buffer. */
            /**
             * @brief Constructs a DataBuffer object.
             * 
             * @param startByte The starting byte index of the buffer.
             * @param nBytes The number of bytes in the buffer.
             */
            DataBuffer(std::size_t startByte, std::size_t nBytes)
                : DataBufferBase(startByte, nBytes)
            {
                buffer = std::make_shared<std::vector<DTYPE>>(nBytes);
                nElements = nBytes / sizeof(DTYPE);
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
            std::shared_ptr<std::vector<DTYPE>> getTypedBuffer() const {
                return buffer; 
            }

            /**
             * @brief Overloads the [] operator to access elements in the buffer.
             * 
             * @param idx The index of the element to access.
             * @return A reference to the element at the specified index.
             */
            DTYPE& operator[](std::size_t idx) {
                return buffer.get()[idx];
            }

            /**
             * @brief Clears the buffer by setting all elements to zero.
             */
            void clearBuffer() override {
                buffer->clear();
                nElements = 0;
                startByte = 0;
                nBytes = 0;
            }

            /**
             * @brief Retrieves the number of elements in the buffer.
             * 
             * @return The number of elements in the buffer.
             */
            int getNElements() {
                return nElements;
            }

            /**
             * @brief Retrieves the value at the specified index in the buffer.
             * 
             * @param idx The index of the value to retrieve.
             * @return The value at the specified index.
             */
            double getDoubleValueAt(std::size_t idx) {
                return static_cast<double>(buffer->at(idx));
            }
            
            

            
            
    };
};