#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <set>
#include <algorithm>

using namespace std;

//This one works just fine
//~ const char * data = "tsv/1k5L.tsv";

//This one works just fine
const char * data = "tsv/retail.tsv";

//For these datasets, the bottleneck is candidate generation and superset testing.. So many frequent to compare with.
//~ const char * data = "tsv/simple_mushroom.tsv";
//~ const char * data = "tsv/simple_short.tsv";


int numTransactions;
int minsup;
int rareminsup;
ifstream input;

typedef set< int > ItemSet;
typedef set< ItemSet > SuperSet;

inline bool subset (ItemSet b, ItemSet a){
	return includes(a.begin(), a.end(), b.begin(), b.end());
}

//io functions:
string printSet(ItemSet nums){
	string res = "{";
	int len = nums.size(); //number of elements in itemset
	int i = 0;
	
	for (auto v = nums.begin(); v != nums.end(); v++){
		res += to_string(*v);
		if (++i != len)
			res += ",";
	}
	return res + "}";
}

void printSuperSet(const SuperSet& s, std::ofstream& out){
	for (auto i: s){
		out << printSet(i);
	}
}


int getNumTransactions(){
	if (input.is_open())
		input.close();
	input.open(data, ifstream::in);
	string line;
	
	int num;
	getline(input, line);//discard first line.
	istringstream parser(line);
	parser >> num;
	
	input.close();
	return num;
}

ItemSet transactionToSet(string line){
	ItemSet res;
	int token;
	istringstream parser(line);
	for (int i = 0; parser >> token; i++){
		res.insert(token);
	}
	
	return res;
}

void resetTransactionFile(){
	if (input.is_open())
		input.close();
	input.open(data, ifstream::in);
	string line;
	
	getline(input, line);//discard first line.
}

ItemSet getTransaction(){
	string line;
	if (getline(input, line)){
		return transactionToSet(line);
	
	} else {
		ItemSet x;
		return x;
	}
}


// generate L1 from database
SuperSet getL1(){
	map<int, int> support;
	
	resetTransactionFile();
	ItemSet transaction = getTransaction();
	while (! transaction.empty()){
		
		for (auto v = transaction.begin(); v != transaction.end(); v++){
			support[*v]++;
		}
		transaction = getTransaction();
	}
	
	SuperSet L1;
	for(auto i = support.begin(); i!= support.end(); ++i){
		if (i->second > minsup){
			ItemSet x;
			x.insert(i->first);
			L1.insert(x);
		}
	}
	
	return L1;
}

ItemSet setUnion(ItemSet b, ItemSet a){
	ItemSet c;
	c.insert(b.begin(), b.end());
	c.insert(a.begin(), a.end());
	return c;
}

SuperSet genCandidates (SuperSet frequent){
	SuperSet res;
	
	//sum i = 1 to n (i) == n(n+1)/2
	double possibleCombos = frequent.size() * (frequent.size() - 1) /2;
	
	int counter = 0;
	//Estimate number of steps total from transactionCount * candidateCount
	cout <<"Generating candidates, max number of candidates: "<<possibleCombos << endl;
	
	for (auto i = frequent.begin(); i != frequent.end(); i++){
		
		//Iterate starting at i + 1...
		auto j = i; 
		++j;
		
		for (j; j != frequent.end(); j++){
			//create union
			ItemSet z = setUnion(*i,*j);
			
			if (++counter % 10000 == 0){
				cout <<"Generated so far: "<<counter / possibleCombos * 100<<"%                  \r";
			}
			
			if (z.size() != i->size()+1) //only want to make supersets ONE bigger.
				continue;
			
			bool infrequent_subset = false;
			//create subsets for pruning phase:
			for (auto remove_item = z.begin(); remove_item!= z.end(); remove_item++){
				ItemSet temp = z;
				temp.erase(temp.find(*remove_item));
				
				//Subset is rare if temp != A, temp != B, and temp not any other frequent subset of them combined.
				if ((*i) != temp  && (*j) != temp && frequent.find(temp) == frequent.end()){
					infrequent_subset = true;
					break;
				}
				//check all subsets are themselves frequent...
			}
			
			if (!infrequent_subset){
				res.insert(z); //no evidence this set is not frequent yet.
			} else {
			}
			
		}
	}
	
	cout <<"Potentially Frequent Candidates after pruning: "<<res.size() << endl;
	
	return res;
}

SuperSet verify (const SuperSet & candidates, SuperSet & rareGenerators){
	rareGenerators.clear();
	map<ItemSet, int> support;
	
	resetTransactionFile();
	ItemSet transaction = getTransaction();
	
	int counter = 0;
	//Estimate number of steps total from transactionCount * candidateCount
	
	double testsRequired = numTransactions * candidates.size();
	cout <<"Subset tests required: "<<testsRequired << endl;
	
	while (! transaction.empty()){
		for (auto candidate : candidates){
			if (++counter % 10000 == 0){
				cout <<"Subset tests so far: "<<counter / testsRequired * 100<<"%                  \r";
			}
			
			if ( subset (candidate, transaction)){
				support[candidate]++;
			}
		}
		transaction = getTransaction();
	}
	
	cout <<"                                                                                                            ";
	
	SuperSet result;
	
	for (auto i: support){
		if (i.second >= minsup){
			result.insert(i.first);
		} else if (i.second <= minsup && i.second > rareminsup){
			rareGenerators.insert(i.first);
		}
	}
	
	return result;
}

int main(){
	cout <<endl << "Transaction File: " << data << endl;
	numTransactions = getNumTransactions();
	//~ minsup = 10;
	//~ minsup = numTransactions * 0.50;
	minsup = numTransactions * 0.02;
	rareminsup = numTransactions * 0.001;
	
	cout <<"Number of transactions: "<<numTransactions<<endl;
	cout <<"Minimum support       : "<<minsup<<endl;
	cout <<"Rare minimum support  : "<<rareminsup<<endl;
	
	//~ exit(0);
	ofstream out;
	out.open("out.output", ofstream::out);
	
	SuperSet verified = getL1();
	out <<"Frequent Singletons: "<< verified.size() <<endl;
	printSuperSet(verified, out);
	
	cout << verified.size() << " singletons have the minimum support"<<endl;
	
	int level = 2;
	while (verified.size() > 0){
		cout <<"\nLevel "<<level << endl;
			
		cout <<verified.size() << " itemsets to make candidates from." << endl;
		SuperSet candidates = genCandidates(verified);
		
		SuperSet rareGenerators;
		verified = verify(candidates, rareGenerators);
		
		out <<"\n\nLevel "<<level << " frequent itemsets: "<< verified.size()<<endl;
		printSuperSet(verified, out);
		
		out <<"\n\nLevel "<<level << " minimal rare itemsets: "<< rareGenerators.size()<<endl;
		printSuperSet(rareGenerators, out);
		//save result to output file:
		
		level++;
	}
	
	out.close();
	
	cout <<endl <<"All processing complete"<<endl;
	return 0;
}
