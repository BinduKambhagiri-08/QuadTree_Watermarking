#include "../include/ApiServer.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    int port = 8080;
    std::string frontendPath = "./frontend";
    std::string outputPath = "./output";
    std::string inputPath = "./input";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--frontend" && i + 1 < argc) {
            frontendPath = argv[++i];
        } else if (arg == "--output" && i + 1 < argc) {
            outputPath = argv[++i];
        } else if (arg == "--input" && i + 1 < argc) {
            inputPath = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: QuadTreeWatermark [options]\n"
                      << "Options:\n"
                      << "  --port <number>      HTTP Server Port (default: 8080)\n"
                      << "  --frontend <path>    Frontend directory (default: ./frontend)\n"
                      << "  --output <path>      Output directory (default: ./output)\n"
                      << "  --input <path>       Input directory (default: ./input)\n"
                      << "  --help, -h           Show this help message\n";
            return 0;
        }
    }

    try {
        ApiServer server(port, frontendPath, outputPath, inputPath);
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "[Fatal Error] Exception in server: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
