make: main

run: main
	./main
	
main: main.cpp
	g++ -std=c++11 main.cpp -o main

clean:
	rm -f main
