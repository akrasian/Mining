#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <set>
#include <algorithm>

using namespace std;

//~ const char * data = "tsv input/simple_short.tsv";
//~ const char * data = "tsv/1k5L.tsv";
const char * data = "tsv/retail.tsv";

int numTransactions = 88163;
int minsup = 10000;
int rareminsup = 100;

typedef set< int > ItemSet;
typedef set< ItemSet > SuperSet;

inline bool subset (ItemSet b, ItemSet a){
	return includes(a.begin(), a.end(), b.begin(), b.end());
}

//io functions:
string printSet(ItemSet nums){
	string res = "{";
	for (auto v = nums.begin(); v != nums.end(); v++){
		res += to_string(*v);
		res += ",";
	}
	return res + "}";
}

ifstream input;

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
	
	for (auto i = frequent.begin(); i != frequent.end(); i++){
		auto j = i; 
		++j;
		
		for (j; j != frequent.end(); j++){
			//create union
			ItemSet z = setUnion(*i,*j);
			
			if (z.size() != i->size()+1) //only want to make supersets ONE bigger.
				continue;
			
			bool infrequent_subset = false;
			//create subsets for pruning phase:
			for (auto i = z.begin(); i!= z.end(); i++){
				ItemSet temp = z;
				temp.erase(temp.find(*i));
				
				if (frequent.find(temp) == frequent.end()){
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
	
	return res;
}

SuperSet verify (const SuperSet & candidates, SuperSet & rareGenerators){
	rareGenerators.clear();
	map<ItemSet, int> support;
	
	resetTransactionFile();
	ItemSet transaction = getTransaction();
	while (! transaction.empty()){
		
		for (auto candidate : candidates){
			if ( subset (candidate, transaction)){
				support[candidate]++;
			}
		}
		transaction = getTransaction();
	}
	
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

void printSuperSet(const SuperSet& s, std::ofstream& out){
	for (auto i: s){
		out << printSet(i) << endl;
	}
}

int main(){
	SuperSet verified = getL1();
	
	cout << verified.size() << " singletons have minimal support"<<endl;
	
	ofstream out;
	out.open("out.output", ofstream::out);
	
	int level = 2;
	while (verified.size() > 0){
		cout <<"Level "<<level << endl;
		
		cout <<verified.size() << " itemsets to make candidates from." << endl;
		SuperSet candidates = genCandidates(verified);
		cout <<candidates.size() << " candidates to test" << endl;
		
		SuperSet rareGenerators;
		verified = verify(candidates, rareGenerators);
		
		out <<"\n\nLevel "<<level << " frequent itemsets"<<endl;
		printSuperSet(verified, out);
		
		out <<"\n\nLevel "<<level << " minimal rare itemsets"<<endl;
		printSuperSet(rareGenerators, out);
		//save result to output file:
		
		level++;
	}
	cout <<"verified has no more transactions" << endl;
	
	out.close();
	
	cout <<"All processing complete"<<endl;
	return 0;
}
