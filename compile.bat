mkdir obj
g++ -Wall -O3 -std=c++17 -I./lib/mingw-std-threads-master -IE:\Privat\Programmieren\Klassen\high_resolution_timer -c main.cpp -o obj/main.o
g++ -Wall -O3 -std=c++17 -I./lib/mingw-std-threads-master -c event_src/event_handler_src.cpp -o obj/event_handler_src.o
g++ -Wall -O3 -std=c++17 -I./lib/mingw-std-threads-master -c event_src/listener_src.cpp -o obj/listener_src.o

g++ -o event_handler_test.exe obj/main.o obj/event_handler_src.o obj/listener_src.o -s

ar -r lib_event_handler.a obj/event_handler_src.o obj/listener_src.o -s