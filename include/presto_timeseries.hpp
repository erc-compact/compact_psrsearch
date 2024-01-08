#include "search_mode_file.hpp"

namespace FileTypes{

    class PrestoTimeSeries : public SearchModeFile{
        public:
            PrestoTimeSeries(std::string filename);
            ~PrestoTimeSeries();
            
            void write_header();
            void read_header();

            int read_data();
            int write_data();


           
    };
};
