rsocket:
gcc -g -c rsocket.c -o rsocket.o -pthread
ar rcs librsocket.a rsocket.o
clean:
rm rsocket.o librsocket.a
user1:
gcc -g -c rsocket.c -o rsocket.o -pthread 
ar rcs librsocket.a rsocket.o 
gcc -g -c -lpthread user1.c -o user1.o -pthread
gcc -g -o user1 user1.o -L. -lrsocket -pthread
./user1
user2:
gcc -g -c rsocket.c -o rsocket.o -pthread 
ar rcs librsocket.a rsocket.o 
gcc -g -c -lpthread user2.c -o user2.o -pthread 
gcc -g -o user2 user2.o -L. -lrsocket -pthread
./user2
