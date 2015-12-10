all: fpgrowth
#~ all: apriori

#~ run: runApriori
run: runFPgrowth

runApriori: apriori
#~ 	./apriori tsv/tiny.tsv tiny.txt 0.6
	./apriori tsv/1k5L.tsv 1k5L.txt 0.01 1

runFPgrowth: fpGrowth
#~ 	./fpGrowth tsv/tiny.tsv tiny.txt 0.6
#~ 	./fpGrowth tsv/simple_short.tsv simple_short.txt 0.6
#~ 	./fpGrowth tsv/simple_short.tsv simple_short.txt 0.6
	./fpGrowth tsv/1k5L.tsv tsv/1k5L_fp.txt 0.002 1

#~ 	./arima

apriori: apriori.cpp
	g++ -std=c++11 apriori.cpp -o apriori

fpgrowth: fpGrowth.cpp
	g++ -std=c++11 fpGrowth.cpp -o fpGrowth
	
clean:
	rm -f apriori fpgrowth
