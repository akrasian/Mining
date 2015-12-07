make: main

run: main
	./main tsv/c20d10k.tsv c20d10k.txt 0.95 1

main: main.cpp
	g++ -std=c++11 main.cpp -o main

clean:
	rm -f main
