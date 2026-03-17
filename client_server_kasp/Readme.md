Как забилдить:
	set(GTEST_LIBRARIES gtest gtest_main)
(В папке build)
cmake ..
make
(сервер)
./server <config json> <port>
(клиент)
./client <filepath> <port>
(статистика)
./stats
