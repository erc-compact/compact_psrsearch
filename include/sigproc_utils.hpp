int parse_angle_str(const std::string& angle_str, int& first, int& second, double& third);
void parse_angle_sigproc(double sigproc, int& sign, int& first, int& second, double& third);
void sigproc_to_hhmmss(double sigproc, std::string& hhmmss);
void sigproc_to_ddmmss(double sigproc, std::string& ddmmss);
void ddmmss_to_sigproc(const std::string& ddmmss, double& sigproc);
void hhmmss_to_sigproc(const std::string& hhmmss, double& sigproc);
int hhmmss_to_double(const std::string& hhmmss, double& degree_value);
int ddmmss_to_double(const std::string& ddmmss, double& degree_value);