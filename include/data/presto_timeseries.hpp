#include "data/search_mode_file.hpp"

namespace IO{

    typedef float PRESTO_DAT_TYPE;

    class PrestoTimeSeries : public SearchModeFile{
        public:
            PrestoTimeSeries(std::string fileName, std::string mode);
            virtual ~PrestoTimeSeries() = default;


            bool isHeaderSeparate();

            void readHeader();
            void writeHeader();

            void readAllData();
            void writeAllData();

            void readNBytes(std::size_t startByte, std::size_t nBytes); 
            void writeNBytes();

        private:
            void readInfLineValStr(std::ifstream& infofile, std::string& valstr, const std::string& errdesc);


           
    };
};
