#ifndef CACHE_H
#define CACHE_H

#include <unordered_map>
#include <vector>
#include <mutex>

struct CachePage {
    int file_descriptor;
    off_t block_offset;
    std::vector<char> data;
    bool referenced;
    bool dirty;
};

// Класс блочного кэша
class BlockCache {
public:
    explicit BlockCache(size_t max_pages);
    ~BlockCache();

    std::vector<char> read(int fd, off_t block_offset);
    void write(int fd, off_t block_offset, const std::vector<char>& data);
    void flush(int fd);

private:
    size_t max_pages_;
    std::vector<CachePage> pages_;
    std::unordered_map<int, size_t> page_map_; 
    std::mutex mutex_;  

    size_t find_victim();
    void load_page(int fd, off_t block_offset);
};

#endif 
