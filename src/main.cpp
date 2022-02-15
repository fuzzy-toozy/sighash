
#include "digest/digest.h"
#include "job/job_controller.h"
#include "job/signature_calculator_job.h"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

bool ProessCMD(int argc, char **argv,
               std::filesystem::path &source_file,
               std::filesystem::path &destination_file,
               size_t &block_size)
{

    try
    {
        po::options_description desc("Program Usage", 1024, 512);
        desc.add_options()("help,h", "Display help message")("source_file,sf", po::value<std::filesystem::path>(&source_file)->required(), "Source file")("destination_file,df", po::value<std::filesystem::path>(&destination_file)->required(), "Destination file")("block_size,bs", po::value<size_t>(&block_size)->default_value(job::gkDefaultBlockSize), "Hash block size");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if (vm.count("help"))
        {
            std::cout << desc << std::endl;
            return false;
        }
        po::notify(vm);
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return false;
    }

    return true;
}

int main(int argc, char **argv)
{

    std::filesystem::path source_file, destination_file;
    size_t block_size{0};
    if (!ProessCMD(argc, argv, source_file, destination_file, block_size))
    {
        return -1;
    }

    job::JobController controller;

    if (!controller.DoJob(std::make_unique<job::SignatureCalculatorJob<digest::DigestMD5>>(source_file, destination_file, block_size)))
    {
        std::cout << "Failed to complete signature calculation" << std::endl;
        return -1;
    }

    return 0;
}