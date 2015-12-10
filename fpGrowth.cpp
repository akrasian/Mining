#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <climits>
#include <list>

using namespace std;

const char * dataFile;
const char * outputFile;
int numTransactions;
int minsup;
ifstream input;

typedef set< int > ItemSet;

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

string intToLetter(int input){
	char c =  (char)(input) + 64;
	char s [2];
	s[0] = c;
	s[1] = '\0';
	string x = string(s);
	return x;
}

void resetTransactionFile(){
	if (input.is_open())
		input.close();
	input.open(dataFile, ifstream::in);
	string line;
	
	getline(input, line);//discard first line.
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

ItemSet getTransaction(){
	string line;
	if (getline(input, line)){
		return transactionToSet(line);
	
	} else {
		ItemSet x;
		return x;
	}
}


class Node {
public:
	
	Node * parent;
	map <int, Node* > children;
	size_t count;
	int id;
	
	Node (Node * p, int count_start){
		parent = p;
		count = count_start;
	}
	
	~Node(){
		for (auto child : children){
			delete (child.second);
		}
	}
	
	void insert(list<int> & transaction, map <int, set <Node *> > & instances){
		cout <<"Got insert in "<<id<<endl;
		count++;
		
		if (transaction.size() == 1){
			id = transaction.front();
			transaction.pop_front();
			
		} else if(transaction.size() > 1){
			//~ cout <<"Here"<<endl;
			
			id = transaction.front();
			transaction.pop_front();
			
			int child = transaction.front();
			//~ auto p = 
			auto f= children.find(child);
			
			Node * child_ptr = nullptr;
			if (f == children.end()){
				cout <<"Creating CHild"<<endl;
				//Children get this as the parent.
				child_ptr = new Node(this, 0);
				children[child] = child_ptr;
				instances[child].insert(child_ptr);
			} else {
				child_ptr = children[child];
			}
			
			
			if(child_ptr != nullptr){
				child_ptr->insert(transaction, instances);
			}
		}
	}
	
	void print (int depth){
		std::cout << std::string(depth, ' ') << intToLetter(id) << ": "<<count << endl;
		for (auto child : children){
			(child.second)->print(depth+3);
		}
	}
	
	void climb_r (list<int> & branch){
		if (parent == nullptr) 
			return;
		
		branch.push_front(id);
		parent->climb_r(branch);
	}
	
	int climb(list<int> & branch){
		if (parent == nullptr)
			return count;
		
		parent->climb_r(branch);
		
		return count;
	}
};

class Tree {
public:
	map <int, set <Node *> > instances;
	Node * root;
	
	Tree(){
		root = new Node(nullptr, 0);
		cout <<"Making tree"<<endl;
	}
	
	void insert(list<int> & transaction){
		transaction.push_front(0);
		root->insert(transaction, instances);
	}
	
	void print (){
		root->print(0);
	}
	
	~Tree (){
		delete (root);
	}
	
	void conditional(){
		auto itemNodeSet = instances[1];
		
		//~ for (auto itemNodeSet : instances){
			//Generate the conditional DB containing this value:
			Tree temp;
			
			//~ for(auto itemNode : itemNodeSet.second){
			for(auto itemNode : itemNodeSet){
				//Iterate over leafs with this value
				list<int> pathUp;
				int count = itemNode->climb(pathUp);
				
				cout << intToLetter(itemNode->id);
				cout << "A count == "<< count << endl;
	
				for( int i = 0; i< count; i++){
					cout <<"INserting"<<endl;
					temp.insert(pathUp);
				}
				
			}
			
			
			cout <<"Conditional on "<<intToLetter(1) << endl;
			temp.print();
			cout << endl;
			
		//~ }
	}
};

bool comparePairVal (pair<int, int> a, pair<int, int> b){
	return a.first > b.first;
}

bool comparePairFreq (pair<int, int> a, pair<int, int> b){
	return a.second < b.second;
}

Tree parseDB(){
	Tree result;
	
	//First DB pass - get freqs
	resetTransactionFile();
	ItemSet transaction = getTransaction();
	map <int, int> singletonFrequency;
	
	while (! transaction.empty()){
		
		for (auto v = transaction.begin(); v != transaction.end(); v++){
			singletonFrequency[*v]++;
		}
		transaction = getTransaction();
	}
	
	map<int, int> frequency;
	for (auto x: singletonFrequency){
		if (x.second >= minsup)
			frequency[x.first] = x.second;
	}
	
	//Second DB pass - build tree:
	
	resetTransactionFile();
	transaction = getTransaction();
	
	while (! transaction.empty()){
		vector<pair<int, int>> pairs;
		
		for (auto v = transaction.begin(); v != transaction.end(); v++){
			//Create a list of <value, frequency> from transaction.
			auto f= frequency.find(*v);
			if (f != frequency.end()){
				pair<int, int> x;
				x.first = *v;
				x.second = f->second;
				pairs.push_back(x);
			}
		}
		
		stable_sort (pairs.begin(), pairs.end(), comparePairVal);
		stable_sort (pairs.begin(), pairs.end(), comparePairFreq);
		
		list<int> sortedItems;
		
		for(int i = pairs.size()-1; i >=0; --i){
			sortedItems.push_back(pairs[i].first);
		}
		
		cout <<"Inserting"<<endl;
		result.insert(sortedItems);
		
		cout << endl << endl;
		transaction = getTransaction();
		
	}
	
	return result;
};

void fpgrowth(){
	Tree DB = parseDB();
	DB.print();
	DB.conditional();
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
	
	fpgrowth();
	//~ apriori();
	
	cout <<endl <<"All processing complete"<<endl;
	return 0;
}
