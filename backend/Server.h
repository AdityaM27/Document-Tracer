#ifndef DOCUTRACE_SERVER_H
#define DOCUTRACE_SERVER_H

#include <string>
#include <memory>
#include "../include/AnalysisService.h"
#include "../third_party/httplib.h"

namespace docutrace {

/**
 * @brief Lightweight C++ REST API Server powered by cpp-httplib.
 * Demonstrates Unit IV: Formatted networking I/O, multithreading, and REST architecture.
 */
class Server {
private:
    std::string host_;
    int port_;
    std::shared_ptr<AnalysisService> service_;
    httplib::Server svr_;
    AnalysisOptions defaultOptions_;

    void setupRoutes();
    void setCorsHeaders(httplib::Response& res);

public:
    explicit Server(
        std::shared_ptr<AnalysisService> service,
        const std::string& host = "0.0.0.0",
        int port = 8080);
    ~Server() = default;

    /**
     * @brief Starts the HTTP server (blocking).
     */
    void start();

    /**
     * @brief Stops the HTTP server.
     */
    void stop();

    int getPort() const noexcept { return port_; }
    const std::string& getHost() const noexcept { return host_; }
};

} // namespace docutrace

#endif // DOCUTRACE_SERVER_H
