# Отчет

P3314 Мухамеджанов Артур Илдусович

Базовый трек, оценка 3. ЛР 2. 

Вариант: Linux Second Chance

## Описание изменений

Реализация кэша - `/app/src/cache.cpp`
Реализация Api -  `/app/src/file_api.cpp`
Адаптированный бэнчмарк -  `/app/src/benchmark.cpp`


## Результаты работы
Запуск build/benchmark_app testfile.bin 1000 (файл размером 1гб)
Бэнчмарк без моего API : Average I/O latency: 0.0951316 ms
Бэнчмарк с моим API : Average I/O latency: 0.001124 ms