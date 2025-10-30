#include <zmq.hpp>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>
#include <thread>

using namespace std;

// Write a 64-bit value in big-endian format
static void write_be_u64(void* dest, uint64_t v) {
    unsigned char* b = static_cast<unsigned char*>(dest);
    for (int i = 0; i < 8; ++i) {
        b[7-i] = static_cast<unsigned char>(v & 0xFF);
        v >>= 8;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <endpoint> <num_messages> <message_size>\n";
        cerr << "Example: " << argv[0] << " tcp://*:5555 10000 256\n";
        return 1;
    }

    string endpoint = argv[1];
    long long num_messages = atoll(argv[2]);
    size_t message_size = static_cast<size_t>(atoll(argv[3]));
    if (num_messages <= 0 || message_size < 8) {
        cerr << "num_messages must be > 0 and message_size must be >= 8\n";
        return 2;
    }

    try {
        zmq::context_t ctx{1};
        zmq::socket_t pub{ctx, zmq::socket_type::pub};
        pub.bind(endpoint);

        // Allow time for subscriber to connect
        this_thread::sleep_for(chrono::seconds(1));

        // Pre-allocate message buffer
        vector<unsigned char> buffer(message_size, 0);

        for (long long i = 0; i < num_messages; ++i) {
            // Get current timestamp in nanoseconds
            uint64_t now_ns = static_cast<uint64_t>(
                chrono::duration_cast<chrono::nanoseconds>(
                    chrono::system_clock::now().time_since_epoch()
                ).count()
            );

            // Write timestamp in big-endian format at start of message
            write_be_u64(buffer.data(), now_ns);

            // Send the message
            zmq::message_t message(buffer.data(), buffer.size());
            pub.send(message, zmq::send_flags::none);

            // sleep for a short duration to avoid flooding
            this_thread::sleep_for(chrono::microseconds(10));

            // Simple progress indicator
            if ((i + 1) % 1000 == 0) {
                cout << "Sent " << (i + 1) << " messages\r" << flush;
            }
        }
        cout << "\nFinished sending " << num_messages << " messages\n";

    } catch (const zmq::error_t& e) {
        cerr << "ZMQ error: " << e.what() << '\n';
        return 4;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << '\n';
        return 5;
    }

    return 0;
}