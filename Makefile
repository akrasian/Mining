all: fpgrowth
#~ all: apriori

#~ run: runApriori
run: runFPgrowth

runApriori: apriori
	./apriori tsv/tiny.tsv tiny.txt 0.6
#~ 	./main tsv/1k5L.tsv 1k5L.txt 0.01 1

runFPgrowth: fpGrowth
	./fpGrowth tsv/tiny.tsv tiny.txt 0.6
#~ 	./main tsv/c20d10k.tsv output/c20d10k.out 0.15 1
#~ 	./arima

apriori: apriori.cpp
	g++ -std=c++11 apriori.cpp -o apriori

fpgrowth: fpGrowth.cpp
	g++ -std=c++11 fpGrowth.cpp -o fpGrowth
	
clean:
	rm -f apriori fpgrowth
