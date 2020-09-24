#include "CDBTo3DTiles.h"
#include "boost/program_options.hpp"
#include <iostream>

int main(int argc, const char *argv[])
{
    boost::filesystem::path dataPath;
    boost::filesystem::path inputPath;
    boost::filesystem::path outputPath;

    try {
        boost::program_options::options_description converterOptions("Converter Options");
        converterOptions.add_options()("help,h", "Display help message")(
            "input,i",
            boost::program_options::value(&inputPath)->required(),
            "CDB input directory")("output,o",
                                   boost::program_options::value(&outputPath)->default_value(outputPath),
                                   "Converter output directory");

        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, converterOptions),
                                      vm);
        boost::program_options::notify(vm);
    } catch (const std::exception &e) {
        std::cout << e.what() << "\n";
        return 0;
    }

    if (!boost::filesystem::exists(inputPath)) {
        std::cout << inputPath << " input path doesn't exist\n";
        return 0;
    }

    GDALAllRegister();
    CDBTo3DTiles converter;
    converter.Convert(inputPath, outputPath);
    return 0;
}
