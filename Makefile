all: main arima

run: main arima
	./main tsv/c20d10k.tsv output.txt 0.5 1
#~ 	./arima

main: main.cpp
	g++ -std=c++11 main.cpp -o main

arima: arima.cpp
	g++ -std=c++11 arima.cpp -o arima

clean:
	rm -f main arima
