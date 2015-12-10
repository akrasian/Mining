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
SuperSet getL1(){
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
	size_t length = frequent.size();
	size_t possibleCombos = length * length /2;
	
	size_t counter = 0;
	//Estimate number of steps total from transactionCount * candidateCount
	cout <<"Generating Candidates\nMax Number of Candidates: "<<possibleCombos << endl;
	
	auto i_end = --frequent.end();
	for (auto i = frequent.begin(); i != --i_end; i++){
		
		//Iterate starting at i + 1...
		auto j = i;
		++j; 
		
		for (j; j != frequent.end(); j++){
			
			//create union
			ItemSet a = i->first;
			ItemSet b = j->first;
			
			ItemSet c = setUnion(a,b);
			
			++counter;
			if (counter % 10000 == 0){
				cout <<"Generated so far        : "<<counter<< "\r";
			}
			
			if (c.size() != a.size()+1) //only want to make supersets ONE bigger.
				continue;
			
			bool infrequent_subset = false;
			//create subsets for pruning phase:
			
			for (auto remove_item = c.begin(); remove_item!= c.end(); remove_item++){
				ItemSet temp = c;
				temp.erase(temp.find(*remove_item));
				
				//Check minimal support amongst all predecessors:
				int pred_support = frequent[temp];
				
				if (pred_support < minsup){
					infrequent_subset = true;
					break;
				}
			}
			
			if (!infrequent_subset){
				res[c] = 0; //no evidence this set is not frequent yet.
			}
		}
	}
	
	cout <<"Potentially Frequent Candidates after pruning: "<<res.size() << endl;
	
	return res;
}

/*
 * Given a pruned set of candidates, we need to check which are truly frequent in the database, and which are rare.
 * For normal pattern mining the rare ones would be discarded, but for rare pattern mining the minimal rare patterns
 * must also be saved and returned.
 */
SuperSet verify (const SuperSet & candidates){
	map<ItemSet, int> trueSupport;
	
	resetTransactionFile();
	ItemSet transaction = getTransaction();
	
	size_t counter = 0;
	//Estimate number of steps total from transactionCount * candidateCount
	
	size_t testsRequired = numTransactions * candidates.size();
	cout <<"Subset tests required: "<<testsRequired << endl;
	
	while (! transaction.empty()){
		for (auto candidate : candidates){
			if (++counter % 10000 == 0){
				cout <<"Subset tests so far  : "<<counter <<"                  \r";
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
		if (i.second >= minsup){
			frequent[i.first] = i.second;
		}
	}
	
	return frequent;
}

void apriori(){
	ofstream out;
	out.open(outputFile, ofstream::out);
	
	SuperSet verified = getL1();
	out <<"1 sets: frequent itemsets: "<< verified.size() <<endl;
	printSuperSet(verified, out);
	
	out << "-----"<<endl; //marks start of next level.
	
	cout << verified.size() << " singletons have the minimum support"<<endl <<endl;
	
	int level = 2;
	while (verified.size() > 0){
		cout <<"Level "<<level << endl;
			
		cout <<verified.size() << " itemsets to make candidates from." << endl;
		SuperSet candidates = genCandidates(verified);
		
		verified = verify(candidates);
		
		out <<level << " sets: frequent itemsets : "<< verified.size()<<endl;
		printSuperSet(verified, out);
		
		out << "-----"<<endl;
		//save result to output file:
		
		level++;
	}
	
	out.close();
}

int main(int argc, char** argv){
	printf("Number of arguments: %d\n", argc-1);
	if (argc < 4){
		printf("Program expects three arguments, <input tsv> <output file> <minsup ratio>\n");
		printf("Example: ./main tsv/retail.tsv output.txt 0.02\n");
		printf("Quitting\n\n");
		exit(1);
	}
	
	float minsup_ratio = atof(argv[3]);
	dataFile = argv[1];
	outputFile = argv[2];
	
	//~ exit(0);
	cout <<endl << "Transaction File       : " << dataFile << endl;
	cout        << "Output File            : "<<outputFile<<endl;
	numTransactions = getNumTransactions();
	
	minsup = numTransactions * minsup_ratio;
	
	cout <<"Number of transactions : "<<numTransactions<<endl;
	cout <<"Minimum support        : "<<minsup<<endl;
	
	apriori();
	
	cout <<endl <<"All processing complete"<<endl;
	return 0;
}
