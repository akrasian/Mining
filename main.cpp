#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <climits>

using namespace std;

const char * dataFile;
const char * outputFile;
int numTransactions;
int minsup;
int rareminsup;
ifstream input;

typedef set< int > ItemSet;
typedef map< ItemSet, int > SuperSet;

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

void printSuperSet(const SuperSet& s, std::ostream& out){
	for (auto i: s){
		//First element is the superset
		out << "(" << printSet(i.first) << ":" << i.second << ")";
	}
	out << endl;
}

int getNumTransactions(){
	if (input.is_open())
		input.close();
	input.open(dataFile, ifstream::in);
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
	input.open(dataFile, ifstream::in);
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
SuperSet getL1(SuperSet & rareGenerators){
	rareGenerators.clear();
	SuperSet L1candidates;
	SuperSet L1;
	
	resetTransactionFile();
	ItemSet transaction = getTransaction();
	
	while (! transaction.empty()){
		
		for (auto v = transaction.begin(); v != transaction.end(); v++){
			ItemSet temp;
			temp.insert(*v);
			L1candidates[temp]++;
		}
		transaction = getTransaction();
	}
	
	for(auto i = L1candidates.begin(); i!= L1candidates.end(); ++i){
		if (i->second >= minsup)
			L1[i->first] = i->second;
		else
			rareGenerators[i->first] = i->second;
	}
	
	return L1;
}

ItemSet setUnion(ItemSet b, ItemSet a){
	ItemSet c;
	c.insert(b.begin(), b.end());
	c.insert(a.begin(), a.end());
	return c;
}

/* Given the previous frequent n-length itemsets, produce all n+1 length length itemset candidates.
 * During this process, there will be three types of candidates generated:
 * Those where at least one subset is infrequent (These cannot be frequent, nor minimally rare)
 * Those where no subset is frequent may be frequent themselves, or they may be infrequent.
 * If no subset is infrequent, but they are frequent, this is a minimally rare itemset.
 * But to tell frequent and minimally rare itemsets apart, we need to return them both and test
 * them against the dataset.
 * 
 * This function also tracks the minimum support of all frequent subsets, in order to determine later if a mRI is a generator.
 */
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
			ItemSet a = i->first;
			ItemSet b = j->first;
			
			ItemSet c = setUnion(a,b);
			
			if (++counter % 10000 == 0){
				cout <<"Generated so far: "<<counter / possibleCombos * 100<<"%                  \r";
			}
			
			if (c.size() != a.size()+1) //only want to make supersets ONE bigger.
				continue;
			
			bool infrequent_subset = false;
			//create subsets for pruning phase:
			
			int lowest_pred_support = INT_MAX;
			
			for (auto remove_item = c.begin(); remove_item!= c.end(); remove_item++){
				ItemSet temp = c;
				temp.erase(temp.find(*remove_item));
				
				//Check minimal support amongst all predecessors:
				int pred_support = frequent[temp];
				
				if (pred_support < minsup){
					infrequent_subset = true;
				}
				
				if (pred_support < lowest_pred_support){
					lowest_pred_support = pred_support;
				}
				//check all subsets are themselves frequent...
			}
			
			if (!infrequent_subset){
				res[c] = lowest_pred_support; //no evidence this set is not frequent yet.
			}
		}
	}
	
	cout <<"Potentially Frequent Generates after pruning: "<<res.size() << endl;
	
	return res;
}

/*
 * Given a pruned set of candidates, we need to check which are truly frequent in the database, and which are rare.
 * For normal pattern mining the rare ones would be discarded, but for rare pattern mining the minimal rare patterns
 * must also be saved and returned.
 */
SuperSet verify (const SuperSet & candidates, SuperSet & rareGenerators){
	//Always empty the passed map first.
	rareGenerators.clear();
	map<ItemSet, int> trueSupport;
	
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
			
			if ( subset (candidate.first, transaction)){
				trueSupport[candidate.first]++;
			}
		}
		transaction = getTransaction();
	}
	
	cout <<"                                                                                                            \n";
	
	SuperSet frequent;
	
	for (auto i: trueSupport){
		ItemSet curr = i.first;
		int pred_min_sup = candidates.at(curr);
		//Minimal rare generators must have lower support than all proper subsets. Candidates tracks the lowest support of all subsets.
		
		if (i.second >= minsup){
			frequent[i.first] = i.second;
		} else if (i.second < pred_min_sup){ // && i.second > rareminsup
			rareGenerators[i.first] = i.second;
		}
	}
	
	return frequent;
}

int main(int argc, char** argv){
	printf("Number of arguments: %d\n", argc-1);
	if (argc < 5){
		printf("Program expects four arguments, <input tsv> <output file> <minsup ratio> <rareminsup integer>\n");
		printf("Example: ./main tsv/retail.tsv output.txt 0.02 1\n");
		printf("Quitting\n\n");
		exit(1);
	}
	
	float minsup_ratio = atof(argv[3]);
	float rareminsup = atoi(argv[4]);
	dataFile = argv[1];
	outputFile = argv[2];
	
	//~ exit(0);
	cout <<endl << "Transaction File       : " << dataFile << endl;
	cout        << "Output File            : "<<outputFile<<endl;
	numTransactions = getNumTransactions();
	
	minsup = numTransactions * minsup_ratio;
	
	cout <<"Number of transactions : "<<numTransactions<<endl;
	cout <<"Minimum support        : "<<minsup<<endl;
	cout <<"Rare minimum support   : "<<rareminsup<<endl;
	
	ofstream out;
	out.open(outputFile, ofstream::out);
	
	SuperSet rareGenerators;
	SuperSet verified = getL1(rareGenerators);
	out <<"1 sets: frequent generators: "<< verified.size() <<endl;
	printSuperSet(verified, out);
	
	out <<"1 sets: minimal rare generators: "<< rareGenerators.size()<<endl;
	printSuperSet(rareGenerators, out);
	
	out << "-----"<<endl; //marks start of next level.
	
	cout << verified.size() << " singletons have the minimum support"<<endl;
	
	int level = 2;
	while (verified.size() > 0){
		cout <<"Level "<<level << endl;
			
		cout <<verified.size() << " itemsets to make candidates from." << endl;
		SuperSet candidates = genCandidates(verified);
		
		verified = verify(candidates, rareGenerators);
		
		out <<level << " sets: frequent generators: "<< verified.size()<<endl;
		printSuperSet(verified, out);
		
		out <<level << " sets: minimal rare generators: "<< rareGenerators.size()<<endl;
		printSuperSet(rareGenerators, out);
		
		out << "-----"<<endl;
		//save result to output file:
		
		level++;
	}
	
	out.close();
	
	cout <<endl <<"All processing complete"<<endl;
	return 0;
}
