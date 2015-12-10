all: main arima

run: main arima
	./main tsv/retail.tsv output.txt 0.02 1
#~ 	./main tsv/1k5L.tsv 1k5L.txt 0.01 1
#~ 	./main tsv/c20d10k.tsv output/c20d10k.out 0.15 1
#~ 	./arima

main: main.cpp
	g++ -std=c++11 main.cpp -o main

arima: arima.cpp
	g++ -std=c++11 arima.cpp -o arima

clean:
	rm -f main arima
