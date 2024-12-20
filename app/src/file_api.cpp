#include "file_api.h"
#include "cache.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <unordered_map>

static BlockCache cache(100); 
static std::unordered_map<int, off_t> file_offsets; 

int lab2_open(const char* path) {
    int fd = open(path, O_RDWR | O_DIRECT);
    if (fd < 0) {
        throw std::runtime_error("Ошибка открытия файла");
    }
    file_offsets[fd] = 0; 
    return fd;
}

int lab2_close(int fd) {
    if (file_offsets.find(fd) != file_offsets.end()) {
        cache.flush(fd);
        file_offsets.erase(fd);
    }
    return close(fd);
}

ssize_t lab2_read(int fd, void* buf, size_t count) {
    if (file_offsets.find(fd) == file_offsets.end()) {
        throw std::invalid_argument("Файл не открыт");
    }

    size_t total_read = 0;
    char* output_buffer = static_cast<char*>(buf);

    while (total_read < count) {
        off_t current_offset = file_offsets[fd];
        off_t block_offset = (current_offset / 4096) * 4096;
        size_t block_start = current_offset % 4096;

        size_t to_read = std::min<size_t>(count - total_read, 4096 - block_start);

        auto data = cache.read(fd, block_offset);

        memcpy(output_buffer + total_read, data.data() + block_start, to_read);

        total_read += to_read;
        file_offsets[fd] += to_read;
    }

    return total_read;
}


ssize_t lab2_write(int fd, const void* buf, size_t count) {
    if (file_offsets.find(fd) == file_offsets.end()) {
        throw std::invalid_argument("Файл не открыт");
    }
    off_t block_offset = (file_offsets[fd] / 4096) * 4096;
    std::vector<char> data(static_cast<const char*>(buf), static_cast<const char*>(buf) + count);
    cache.write(fd, block_offset, data);
    file_offsets[fd] += count;
    return count;
}

off_t lab2_lseek(int fd, off_t offset, int whence) {
    if (file_offsets.find(fd) == file_offsets.end()) {
        throw std::invalid_argument("Файл не открыт");
    }
    if (whence == SEEK_SET) {
        file_offsets[fd] = offset;
    } else if (whence == SEEK_CUR) {
        file_offsets[fd] += offset;
    } else {
        throw std::invalid_argument("Некорректный аргумент whence");
    }
    return file_offsets[fd];
}

int lab2_fsync(int fd) {
    if (file_offsets.find(fd) == file_offsets.end()) {
        throw std::invalid_argument("Файл не открыт");
    }
    cache.flush(fd);
    return fsync(fd);
}
