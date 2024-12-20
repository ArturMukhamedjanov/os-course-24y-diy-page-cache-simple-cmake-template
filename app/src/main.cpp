#include "file_api.h"
#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    const char* filename = "test_file.bin";

    try {
        struct stat file_stat;
        if (stat(filename, &file_stat) != 0) {
            int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0666);
            if (fd < 0) {
                throw std::runtime_error("Не удалось создать файл");
            }
            close(fd);
            std::cout << "Файл " << filename << " был создан." << std::endl;
        }

        std::cout << "[lab2_open] Открываем файл: " << filename << std::endl;
        int fd = lab2_open(filename);

        const char* data = "Hello, BlockCache!";
        std::cout << "[lab2_write] Записываем данные в файл: \"" << data << "\"" << std::endl;
        lab2_write(fd, data, strlen(data));
        std::cout << "[lab2_lseek] Перемещаем указатель на начало файла." << std::endl;
        lab2_lseek(fd, 0, SEEK_SET);

        char read_buffer[64] = {};
        std::cout << "[lab2_read] Читаем данные из файла." << std::endl;
        lab2_read(fd, read_buffer, strlen(data));
        std::cout << "Прочитано из файла: \"" << read_buffer << "\"" << std::endl;

        std::cout << "[lab2_fsync] Синхронизируем данные с диском." << std::endl;
        lab2_fsync(fd);

        std::cout << "[lab2_close] Закрываем файл." << std::endl;
        lab2_close(fd);

    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
