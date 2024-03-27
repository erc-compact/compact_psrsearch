#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <vector>

uint8_t extractBitsFromByte(uint8_t byte, uint8_t b1, uint8_t b2)
{
    // Create a mask with ones in the positions from b1 to b2
    uint8_t mask = ((1 << (b2 - b1 + 1)) - 1) << b1;

    // Apply the mask to the byte and shift the result to the rightmost position
    uint8_t result = (byte & mask) >> b1;

    return result;
}

/**
 * Parses an angle string and extracts the values for first, second, and third.
 *
 * @param angle_str The angle string to parse.
 * @param first     The first value extracted from the angle string.
 * @param second    The second value extracted from the angle string.
 * @param third     The third value extracted from the angle string.
 * @return          Returns 0 if the angle string was successfully parsed, otherwise returns a non-zero value.
 */
int parse_angle_str(const std::string& angle_str, int& first, int& second, double& third)
{
    std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss(angle_str);

    while (std::getline(ss, token, ':')) {
        tokens.push_back(token);
    }

    if (tokens.size() != 3) {
        return EXIT_FAILURE;
    }

    first = std::stoi(tokens[0]);
    second = std::stoi(tokens[1]);
    third = std::stod(tokens[2]);

    return EXIT_SUCCESS;
}

/**
 * @brief Parses the angle in sigproc format.
 *
 * This function takes a sigproc angle and parses it into its components.
 *
 * @param sigproc The sigproc angle to be parsed.
 * @param sign The sign of the angle (-1 for negative, 1 for positive).
 * @param first The first component of the angle.
 * @param second The second component of the angle.
 * @param third The third component of the angle.
 */
void parse_angle_sigproc(double sigproc, int& sign, int& first, int& second, double& third)
{

    sign = sigproc < 0 ? -1 : 1;
    double abs_sigproc = std::abs(sigproc);

    first = (int)(abs_sigproc / 1e4);
    second = (int)((abs_sigproc - first * 1e4) / 1e2);
    third = abs_sigproc - first * 1e4 - second * 1e2;

}


/**
 * Converts a sigproc value to HH:MM:SS format.
 *
 * @param sigproc The sigproc value to convert. This has the format of HHMMSS
 * @param hhmmss  The resulting HH:MM:SS string.
 */
void sigproc_to_hhmmss(double sigproc, std::string& hhmmss)
{ 
    int sign, hh, mm;
    double ss;
    parse_angle_sigproc(sigproc, sign, hh, mm, ss);

    std::stringstream sstream;
    sstream << std::setw(2) << std::setfill('0') << std::fixed << std::setprecision(0) << hh;
    sstream << ":";
    sstream << std::setw(2) << std::setfill('0') << std::fixed << std::setprecision(0) << mm;
    sstream << ":";
    sstream << std::setw(2) << std::setfill('0') << std::fixed << std::setprecision(2) << ss;

    hhmmss = sstream.str();
   
    std::cerr << "sigproc: " << sigproc << " hhmmss: " << hhmmss << std::endl;
}

/**
 * Converts a signal processing value to a string representation in the format "dd:mm:ss".
 * 
 * @param sigproc The signal processing value to convert.
 * @param ddmmss  The resulting string representation in the format "dd:mm:ss".
 */
void sigproc_to_ddmmss(double sigproc, std::string& ddmmss)
{
    int sign, dd, mm;
    double ss;
    parse_angle_sigproc(sigproc, sign, dd, mm, ss);

    std::stringstream sstream;
    if (sign < 0)
    {
        sstream << std::setw(1) << "-";
    }

    sstream << std::setw(2) << std::setfill('0') << std::fixed << std::setprecision(0) << dd;
    sstream << ":";
    sstream << std::setw(2) << std::setfill('0') << std::fixed << std::setprecision(0) << mm;
    sstream << ":";
    sstream << std::setw(2) << std::setfill('0') << std::fixed << std::setprecision(2) << ss;

    ddmmss = sstream.str();
   
    std::cerr << "sigproc: " << sigproc << " ddmmss: " << ddmmss << std::endl;
}



/**
 * @brief Converts a string representation of Declination (dd:mm:ss) to a sigproc value.
 * 
 * @param ddmmss The string representation of ddmmss.
 * @param sigproc The converted sigproc value will be stored in this variable.
 * @return int Returns 0 on success, or a non-zero value on failure.
 */
int ddmmss_to_sigproc(const std::string& ddmmss, double& sigproc)
{
    int ideg = 0;
    int iamin = 0;
    double asec = 0;

    parse_angle_str(ddmmss, ideg, iamin, asec);

    int sign = ideg < 0 ? -1 : 1;
    sigproc = ideg * 1e4 + sign * iamin * 1e2 +  sign * asec;

    return EXIT_SUCCESS;
}


/**
 * @brief Converts a time string in the format "hh:mm:ss" to a sigproc value.
 * 
 * @param hhmmss The time string in the format "hh:mm:ss".
 * @param sigproc The converted sigproc value will be stored in this variable.
 * @return int Returns 0 on success, -1 on failure.
 */
int hhmmss_to_sigproc(const std::string& hhmmss, double& sigproc)
{
    int ihour = 0, imin = 0;
    double sec = 0;
   
   parse_angle_str(hhmmss, ihour, imin, sec);

    char s = '\0';
    if (ihour < 0)
    {
        ihour *= -1;
        s = '-';
    }

    sigproc = ((double)ihour * 1e4 + (double)imin * 1e2 + sec);
    return 0;
}

/**
 * @brief Converts a string representation of time in HH:MM:SS format to a double value in degrees.
 * 
 * @param hhmmss The string representation of time in HH:MM:SS format.
 * @param degree_value The output parameter that will store the converted value in degrees.
 * @return int Returns 0 if the conversion is successful, -1 otherwise.
 */
int hhmmss_to_double(const std::string& hhmmss, double& degree_value)
{
    
    int ihour = 0, imin = 0;
    double sec = 0;
   
   parse_angle_str(hhmmss, ihour, imin, sec);


    if (ihour < 0)
    {
        ihour *= -1;
    }

    degree_value = ((double)ihour * 15.0 + (double)imin * 15.0 / 60.0 + sec * 15.0 / 3600.0);
    return 0;
}


/**
 * @brief Converts a string representation of degrees, minutes, and seconds (ddmmss) to a double value in degrees.
 * 
 * @param ddmmss The string representation of degrees, minutes, and seconds.
 * @param degree_value The output parameter that will hold the converted value in degrees.
 * @return int Returns 0 if the conversion is successful, otherwise returns a non-zero value.
 */
int ddmmss_to_double(const std::string& ddmmss, double& degree_value)
{
    int ideg = 0;
    int iamin = 0;
    double asec = 0;
    parse_angle_str(ddmmss, ideg, iamin, asec);

    if (ideg < 0)
        degree_value = ((double)ideg - (double)iamin / 60.0) - asec / 3600.0;
    else
        degree_value = ((double)ideg + (double)iamin / 60.0) + asec / 3600.0;

    return 0;
}




