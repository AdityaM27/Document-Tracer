#include "Server.h"
#include <iostream>
#include <memory>
#include <csignal>

std::shared_ptr<docutrace::Server> g_server = nullptr;

void signalHandler(int signum) {
    std::cout << "\n[Document Tracer Server] Interrupt signal (" << signum << ") received. Shutting down...\n";
    if (g_server) {
        g_server->stop();
    }
}

int main(int argc, char* argv[]) {
    int port = 8080;
    std::string host = "0.0.0.0";
    size_t threads = 4;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
        } else if (arg == "--threads" && i + 1 < argc) {
            threads = std::stoul(argv[++i]);
        }
    }

    std::cout << "===============================================================\n"
              << "    DOCUMENT TRACER C++ HTTP BACKEND REST API SERVER           \n"
              << "===============================================================\n"
              << " Host:    http://" << host << ":" << port << "\n"
              << " Workers: " << threads << " thread(s)\n"
              << "===============================================================\n";

    auto refIndex = std::make_shared<docutrace::ReferenceIndex>("data/references");
    auto service = std::make_shared<docutrace::AnalysisService>(refIndex, threads);

    g_server = std::make_shared<docutrace::Server>(service, host, port);

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try {
        g_server->start();
    } catch (const std::exception& e) {
        std::cerr << "Server runtime exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
