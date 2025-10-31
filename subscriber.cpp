// /Users/zachary.harvey/flock_dev/playground/zmq_perf_test.cpp/subscriber.cpp
// Simple ZMQ subscriber to measure pub-sub latency.
// Usage: subscriber <endpoint> <num_messages> <expected_message_size_bytes>
//
// Expects publisher to embed a 64-bit big-endian CPU timestamp in nanoseconds
// since the epoch as the first 8 bytes of each message.

#include <zmq.hpp>
#include <zmq_addon.hpp> 
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>

using namespace std;

static uint64_t read_be_u64(const void* data) {
    const unsigned char* b = static_cast<const unsigned char*>(data);
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i) {
        v = (v << 8) | b[i];
    }
    return v;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cout << "Usage: " << argv[0] << " <endpoint> <num_messages> <message_size> <topic>\n";
        cout << "Example: " << argv[0] << " tcp://127.0.0.1:5555 10000 256\n";
        return 1;
    }
    string topic = argv[4];
    string endpoint = argv[1];
    long long num_messages = atoll(argv[2]);
    size_t expected_size = static_cast<size_t>(atoll(argv[3]));
    if (num_messages <= 0 || expected_size < 8) {
        cout << "num_messages must be > 0 and message_size must be >= 8\n";
        return 2;
    }

    try {
        zmq::context_t ctx{1};
        zmq::socket_t sub{ctx, zmq::socket_type::sub};

        cout << "Connecting to " << endpoint << "...\n";
        sub.connect(endpoint);
        sub.set(zmq::sockopt::subscribe, topic);
        //sub.set(zmq::sockopt::rcvtimeo, 1000); // 1 second receive timeout
        sub.set(zmq::sockopt::linger, 0); // 1 second linger timeout
        cout << "Subscribed to topic '" << topic << "'\n";

        vector<uint64_t> lat_ns;
        lat_ns.reserve(static_cast<size_t>(num_messages));

        for (long long i = 0; i < num_messages; ++i) {
            zmq::message_t msg, topic_message;

            // Receive the topic frame
            std::vector<zmq::message_t> recv_msgs;
            const auto ok = zmq::recv_multipart(sub, std::back_inserter(recv_msgs));
            if (!ok.has_value()) {
                //cout << "Receive interrupted\n";
                continue;
            }

            if (recv_msgs.size() != 2) {
                cout << "Warning: expected 2 frames (topic + message), got " << recv_msgs.size() << "\n";
                continue;
            }
            if (recv_msgs[1].size() != expected_size) {
                // not fatal, but inform
                cout << "Note: received message size " << msg.size()
                     << " differs from expected " << expected_size << '\n';
            }

            uint64_t ts_ns = read_be_u64(recv_msgs[1].data()); // timestamp embedded by publisher (big-endian)
            if (ts_ns == 0) {
                // exit trigger
                break;
            }
            
            // current system time in ns since epoch
            uint64_t now_ns = static_cast<uint64_t>(
                chrono::duration_cast<chrono::nanoseconds>(
                    chrono::system_clock::now().time_since_epoch()
                ).count()
            );

            if (now_ns >= ts_ns) {
                lat_ns.push_back(now_ns - ts_ns);
            } else {
                // clock skew or publisher timestamp in future; store zero and continue
                lat_ns.push_back(0);
                cout << "Warning: clock skew detected (now_ns < ts_ns)\n";
                cout << "Timestamp ns: " << ts_ns << ", now ns: " << now_ns << "\n";
            }
        }

        if (lat_ns.empty()) {
            cout << "No latency samples collected\n";
            return 3;
        }

        // compute stats
        sort(lat_ns.begin(), lat_ns.end());
        uint64_t sum = 0;
        for (auto v : lat_ns) sum += v;
        double mean_ns = static_cast<double>(sum) / lat_ns.size();
        uint64_t min_ns = lat_ns.front();
        uint64_t max_ns = lat_ns.back();
        uint64_t median_ns = lat_ns[lat_ns.size() / 2];
        uint64_t p95_ns = lat_ns[(size_t)((lat_ns.size() * 95) / 100)];
        uint64_t p99_ns = lat_ns[(size_t)((lat_ns.size() * 99) / 100)];
        uint64_t dropped_messages = static_cast<uint64_t>(num_messages) - static_cast<uint64_t>(lat_ns.size());

        auto ns_to_us = [](uint64_t n) -> double { return n / 1000.0; };

        cout << "Samples: " << lat_ns.size() << '\n';
        cout << "Min:    " << ns_to_us(min_ns) << " us\n";
        cout << "Mean:   " << ns_to_us(static_cast<uint64_t>(mean_ns)) << " us\n";
        cout << "Median: " << ns_to_us(median_ns) << " us\n";
        cout << "P95:    " << ns_to_us(p95_ns) << " us\n";
        cout << "P99:    " << ns_to_us(p99_ns) << " us\n";
        cout << "Max:    " << ns_to_us(max_ns) << " us\n";
        cout << "Dropped messages: " << dropped_messages << '\n';

        sub.close();
        ctx.close();

    } catch (const zmq::error_t& e) {
        cout << "ZMQ error: " << e.what() << '\n';
        return 4;
    } catch (const exception& e) {
        cout << "Error: " << e.what() << '\n';
        return 5;
    }



    return 0;
}