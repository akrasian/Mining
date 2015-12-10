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

void generateSubsets(vector< pair < int, int> > frequent){
	//~ if(location >= 0){
		//~ if(addElement){
			//~ prefix = intToLetter(frequent[location].first) + ", " + prefix;
			//~ support = (frequent[location].second < support);
			//~ //This is always decreasing so should be fine to not check.
		//~ }
		//~ 
		//~ if (location == 0){
			//~ cout << prefix << ":" << support << endl;
		//~ } else {
			//~ --location;
			//~ permute(prefix, support, frequent, location, 0);
			//~ permute(prefix, support, frequent, location, 1);
		//~ }
		//~ 
	//~ }
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
		id = -1;
	}
	
	~Node(){
		for (auto child : children){
			delete (child.second);
		}
	}
	
	void insert(list<int> & transaction, map <int, set <Node *> > & instances){
		if (transaction.size() < 1) return;
		
		//~ cout <<"Got insert in "<<id<<endl;
		int child_id = transaction.front();
		transaction.pop_front();
		
		auto f= children.find(child_id);
		
		Node * child_ptr = nullptr;
		if (f == children.end()){
			child_ptr = new Node(this, 0);
			child_ptr->id = child_id;
			child_ptr->count = 0;
			
			children[child_id] = child_ptr;
			instances[child_id].insert(child_ptr);
		} else {
			child_ptr = children[child_id];
			
		}
		
		child_ptr->count++;
		child_ptr->insert(transaction, instances);
	}
	
	void insert(list<int> & transaction, map <int, set <Node *> > & instances, int repetitions){
		if (transaction.size() < 1) return;
		
		int child_id = transaction.front();
		transaction.pop_front();
		
		auto f= children.find(child_id);
		
		Node * child_ptr = nullptr;
		if (f == children.end()){
			child_ptr = new Node(this, 0);
			child_ptr->id = child_id;
			child_ptr->count = 0;
			
			children[child_id] = child_ptr;
			instances[child_id].insert(child_ptr);
		} else {
			child_ptr = children[child_id];
			
		}
		
		child_ptr->count+= repetitions;
		child_ptr->insert(transaction, instances, repetitions);
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
	}
	
	void insert(list<int> & transaction){
		root->insert(transaction, instances);
	}
	
	void insert(list<int> & transaction, int repetitions){
		root->insert(transaction, instances, repetitions);
	}
	
	bool isList(){
		Node * temp = root;
		
		while (true){
			int children_size = temp->children.size();
			if(children_size > 1){
				return false;
			} else if (children_size == 0){
				return true;
			} else {
				auto child = *(temp->children.begin());
				temp = child.second;
			}
		}
		
		
	}
	
	void print (){
		root->print(0);
	}
	
	~Tree (){
		delete (root);
	}
	
	
	//ASSUMPTION - already checked this tree was a list.
	void createFrequentPatterns(){
		Node * temp = root;
		
		vector< pair < int, int> > frequent;
		int frequentItems = 0;
		
		while (true){
			int children_size = temp->children.size();
			
			if(children_size > 1){
				printf("ERROR, should only be called on linear trees.\n");
			} else if (children_size == 0){
				break;
			} else {
				//~ cout <<"Adding..."<<endl;
				auto child = *(temp->children.begin());
				temp = child.second;
				
				if(child.second->count < minsup){
					break; //no more freq children to fidn here.
				} else {
					frequentItems++;
					pair <int, int> itemSupport;
					itemSupport.first = child.first;
					itemSupport.second = child.second->count;
					frequent.push_back(itemSupport);
				}
			}
		}
		
		//~ cout <<"Permuting"<<endl;
		//~ cout <<"Found "<<frequentItems<< " frequent items to make strings from"<<endl;
		generateSubsets(frequent);
		
	}
	
	void conditional(){
		//Instances tracks all variables used in this tree:
		if(instances.size() > 0){
			for (auto itemNodeSet : instances){
				//Generate the conditional DB containing this value:
				Tree temp;
				
				for(auto itemNode : itemNodeSet.second){
					list<int> pathUp;
					int count = itemNode->climb(pathUp);
					temp.insert(pathUp, count);
				}
				
				//~ string state = prefix + intToLetter(itemNodeSet.first);
				
				//~ cout <<"\n\nConditional on "<< state << endl;
				//~ temp.print();
				//~ cout << endl;
				
				if (temp.isList()){
					//~ cout <<"Tree is a list, create all permutations."<<endl;
					temp.createFrequentPatterns();
				} else {
					temp.conditional();
				}
				//~ else {
					//~ cout <<"Recursing."<<endl;
				
				//~ }
				
			}
		}
		
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
		
		result.insert(sortedItems);
		transaction = getTransaction();
		
	}
	
	return result;
};

void fpgrowth(){
	Tree DB = parseDB();
	//~ DB.print();
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
