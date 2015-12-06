make: main

run: main
	./main tsv/retail.tsv output.txt 0.02 1

main: main.cpp
	g++ -std=c++11 main.cpp -o main

clean:
	rm -f main
