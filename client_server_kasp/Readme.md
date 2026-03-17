# Как собрать проект
```
mkdir build
cd build
cmake ..
make
 ```

# Использование 
```
cd bin
(в папке build/bin)
./server ../../example/patterns.json 5000

./client ../../example/large_file.txt 5000

./stats
```

Для генерации больших файлов с набором паттернов есть скрипт [scripts/generate_large_file.py](./scripts/generate_large_file.py)

# Замечания
 * Для имитации сложных вычислений над файлом  после поиска каждого паттерна  процесс засыпает на 2 секунды
* В комментарии к функции ClientHandler::scanFile обьяснено почему было принято решение обрабатывать файл целиком , а не потоком
  
# Сторонние зависимости
Все сторонние библиотеки в папке [thirdparty](./thirdparty)

* [nlohmann](./thirdparty/nlohmann/) : JSON ( https://github.com/nlohmann/json )
* [googletest](./thirdparty/nlohmann/) : googletest( https://github.com/google/googletest )

# Автор 
Однодворцев Матвей
