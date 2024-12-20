#include "cache.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <algorithm>

BlockCache::BlockCache(size_t max_pages) : max_pages_(max_pages) {}

BlockCache::~BlockCache() {
    for (auto& page : pages_) {
        if (page.dirty) {
            pwrite(page.file_descriptor, page.data.data(), page.data.size(), page.block_offset);
        }
    }
}

size_t BlockCache::find_victim() {
    while (true) {
        for (size_t i = 0; i < pages_.size(); ++i) {
            if (!pages_[i].referenced) {
                return i;
            }
            pages_[i].referenced = false;
        }
    }
}

void BlockCache::load_page(int fd, off_t block_offset) {
    std::vector<char> buffer(4096);
    ssize_t bytes_read = pread(fd, buffer.data(), buffer.size(), block_offset);

    if (bytes_read < 0) {
        throw std::runtime_error("Ошибка чтения с диска");
    }

    CachePage page = {fd, block_offset, std::move(buffer), true, false};

    if (pages_.size() < max_pages_) {
        pages_.push_back(std::move(page));
    } else {
        size_t victim_idx = find_victim();
        if (pages_[victim_idx].dirty) {
            pwrite(pages_[victim_idx].file_descriptor, pages_[victim_idx].data.data(),
                   pages_[victim_idx].data.size(), pages_[victim_idx].block_offset);
        }
        pages_[victim_idx] = std::move(page);
    }
}

std::vector<char> BlockCache::read(int fd, off_t block_offset) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = page_map_.find(fd);
    if (it == page_map_.end()) {
        load_page(fd, block_offset);
    }
    return pages_[page_map_[fd]].data;
}

void BlockCache::write(int fd, off_t block_offset, const std::vector<char>& data) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = std::find_if(pages_.begin(), pages_.end(), [fd, block_offset](const CachePage& page) {
        return page.file_descriptor == fd && page.block_offset == block_offset;
    });

    if (it != pages_.end()) {
        it->data = data;
        it->dirty = true;
        it->referenced = true;
    } else {
        if (pages_.size() >= max_pages_) {
            size_t victim_idx = find_victim();
            if (pages_[victim_idx].dirty) {
                pwrite(pages_[victim_idx].file_descriptor, pages_[victim_idx].data.data(),
                       pages_[victim_idx].data.size(), pages_[victim_idx].block_offset);
            }
            pages_[victim_idx] = {fd, block_offset, data, true, true};
        } else {
            pages_.push_back({fd, block_offset, data, true, true});
        }
    }
}

void BlockCache::flush(int fd) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& page : pages_) {
        if (page.file_descriptor == fd && page.dirty) {
            pwrite(fd, page.data.data(), page.data.size(), page.block_offset);
            page.dirty = false;
        }
    }
}
