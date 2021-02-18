mkdir obj
g++ -Wall -O3 -std=c++17 -I./lib/mingw-std-threads-master -I./lib/high_resolution_timer -c main.cpp -o obj/main.o
g++ -Wall -O3 -std=c++17 -I./lib/mingw-std-threads-master -c src/event_handler_src.cpp -o obj/event_handler_src.o
g++ -Wall -O3 -std=c++17 -I./lib/mingw-std-threads-master -c src/listener_src.cpp -o obj/listener_src.o

g++ -s -o event_handler_test.exe obj/main.o obj/event_handler_src.o obj/listener_src.o

ar -s -r libevent_handler.a obj/event_handler_src.o obj/listener_src.o