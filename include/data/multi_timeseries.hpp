
#include <vector>
#include <memory>
#include <string>
namespace IO {
    class MultiTimeSeries {
        private:
            std::vector<float> dmList;
            std::vector<std::unique_ptr<SearchModeFile>> outFiles;
            std::shared_ptr<DEDISP_OUTPUT_TYPE[]> dedispersedData;
            std::size_t totalNSamples; // this is set to all samples or the gulped samples based on whether we are writing to file or not
            int dmListSize;
            std::string outDir;
            std::string outPrefix;
            std::string outSuffix;

        public:
            inline MultiTimeSeries(std::vector<float> dmList, std::size_t totalNSamples, std::string outDir, std::string outPrefix, std::string outSuffix, std::string outputFormat):
                MultiTimeSeries(dmList, totalNSamples){
                
                this->outDir = outDir;
                this->outPrefix = outPrefix;
                this->outSuffix = outSuffix;
                this->outFiles.resize(dmListSize);
                for (std::size_t i = 0; i < dmListSize; i++){
                    std::stringstream outFileName;
                    outFileName << outDir << "/" << outPrefix << "_DM" << std::setprecision(3) << std::fixed << dmList[i] << "_" << outSuffix;
                    SearchModeFile* outFile = SearchModeFile::createInstance(outFileName.str(), WRITE, outputFormat);
                    outFiles.push_back(std::make_unique<SearchModeFile>(outFile));
                }
            }

            inline MultiTimeSeries (std::vector<float> dmList, std::size_t nSamples){
                this->dmList = dmList;
                this->totalNSamples = totalNSamples;
                this->dmListSize = dmList.size();
                this->dedispersedData = std::make_shared<DEDISP_OUTPUT_TYPE>(nSamples * dmListSize * sizeof(DEDISP_OUTPUT_TYPE));
            }

            
            inline std::shared_ptr<DEDISP_OUTPUT_TYPE> getDataPtr() {
                return this->dedispersedData;
            }

            DEDISP_OUTPUT_TYPE& operator()(std::size_t iDm, std::size_t iSample){
                return this->dedispersedData.get()[iDm * this->dmListSize + iSample];
            }

            inline void writeToFile(){
                for (std::size_t i = 0; i < dmListSize; i++){
                    outFiles[i]->writeNBytes(this->dedispersedData.get()[i], this->totalNSamples * sizeof(DEDISP_OUTPUT_TYPE));
                }
            }
            ~MultiTimeSeries();

    };
};