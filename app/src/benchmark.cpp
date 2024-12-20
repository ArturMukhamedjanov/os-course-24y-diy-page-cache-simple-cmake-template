#include "file_api.h"
#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <cstring>

constexpr size_t BLOCK_SIZE = 4096;
constexpr size_t FILE_SIZE = 1024 * 1024 * 512; 

void GenerateTestFile(const std::string &filename) {
    int fd = lab2_open(filename.c_str());
    if (fd < 0) {
        throw std::runtime_error("Error: Failed to create file using lab2_open");
    }

    std::vector<char> buffer(BLOCK_SIZE, 0);
    for (size_t i = 0; i < FILE_SIZE / BLOCK_SIZE; ++i) {
        lab2_write(fd, buffer.data(), buffer.size());
    }
    lab2_fsync(fd);
    lab2_close(fd);
}

void MeasureIOLatency(const std::string &filename, size_t iterations) {
    int fd = lab2_open(filename.c_str());
    if (fd < 0) {
        throw std::runtime_error("Error: Failed to open file using lab2_open");
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, FILE_SIZE / BLOCK_SIZE - 1);

    std::vector<double> latencies;
    std::vector<char> buffer(BLOCK_SIZE, 0);

    for (size_t i = 0; i < iterations; ++i) {
        size_t block_index = dist(gen);
        off_t offset = block_index * BLOCK_SIZE;

        auto start = std::chrono::high_resolution_clock::now();
        lab2_lseek(fd, offset, SEEK_SET);
        lab2_read(fd, buffer.data(), buffer.size());
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> elapsed = end - start;
        latencies.push_back(elapsed.count());
    }

    lab2_close(fd);

    double total_latency = 0;
    for (double latency : latencies) {
        total_latency += latency;
    }
    double average_latency = total_latency / latencies.size();

    std::cout << "Average I/O latency: " << average_latency << " ms\n";
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <filename> <iterations>\n";
        return 1;
    }

    std::string filename = argv[1];
    size_t iterations = std::stoull(argv[2]);

    try {
        std::cout << "Generating test file...\n";
        GenerateTestFile(filename);

        std::cout << "Measuring I/O latency...\n";
        MeasureIOLatency(filename, iterations);

        std::cout << "Test complete.\n";
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
