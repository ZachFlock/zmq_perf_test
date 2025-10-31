// /Users/zachary.harvey/flock_dev/playground/zmq_test/broker.cpp
// Simple ZeroMQ PUB/SUB broker using XSUB/XPUB proxy.
// Build: g++ -std=c++17 broker.cpp -lzmq -pthread -o broker

#include <zmq.hpp>
#include <iostream>
#include <thread>
#include <atomic>
#include <csignal>
#include <chrono>
#include <string>

static std::atomic<bool> g_stop{false};

void signal_handler(int)
{
    g_stop.store(true);
}
int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <xsub_endpoint> <xpub_endpoint>\n";
        std::cerr << "Example: " << argv[0] << " tcp://*:5556 tcp://*:5557\n";
        return 1;
    }

    std::string xsub_endpoint = argv[1];
    std::string xpub_endpoint = argv[2];


    if (argc >= 2) xsub_endpoint = argv[1];
    if (argc >= 3) xpub_endpoint = argv[2];

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try {

        zmq::context_t ctx{1};

        zmq::socket_t xsub{ctx, zmq::socket_type::xsub};
        zmq::socket_t xpub{ctx, zmq::socket_type::xpub};

        // Optional: make XPUB verbose to see subscription messages (0/1)
        int verbose = 0;
        xpub.set(zmq::sockopt::xpub_verbose, 0);

        xsub.bind(xsub_endpoint);
        xpub.bind(xpub_endpoint);

        std::cout << "Broker started.\n";
        std::cout << "  XSUB (publishers) bound to: " << xsub_endpoint << "\n";
        std::cout << "  XPUB (subscribers) bound to: " << xpub_endpoint << "\n";
        std::cout << "Press Ctrl-C to exit.\n";

        // Run proxy in a separate thread so main can monitor signals
        std::thread proxy_thread([&](){
            try {
                zmq::proxy(xsub, xpub);
            } catch (const zmq::error_t& e) {
                // proxy exits when sockets are closed; report if unexpected
                if (!g_stop.load()) {
                    std::cerr << "zmq::proxy error: " << e.what() << "\n";
                }
            }
        });

        // Wait for signal
        while (!g_stop.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::cout << "Shutting down broker...\n";
        // Closing sockets will cause zmq::proxy to return / throw and thread to finish
        xsub.close();
        xpub.close();

        if (proxy_thread.joinable()) proxy_thread.join();
        ctx.close();

        std::cout << "Broker stopped.\n";
    } catch (const zmq::error_t& e) {
        std::cerr << "ZeroMQ error: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}