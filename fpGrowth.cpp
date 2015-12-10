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
typedef pair <int, int> Tuple;

set <string> frequentSets;

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

void generateSubsets_r (string result, int pos, size_t max_support, vector< Tuple > & frequent){
	
	size_t currSup = frequent[pos].second;
	size_t currItem = frequent[pos].first;
	
	string result2;
	//Case - adding this element to the string.
	if(result == ""){
		result2 = "{" + std::to_string(currItem) + ":" + std::to_string(currSup);
	} else {
		result2 = result + ", " + std::to_string(currItem) + ":" + std::to_string(currSup);
	}
	
	if (pos <= 0){
		if (result != ""){
			result = result + "}:" + to_string(max_support);
			frequentSets.insert(result);
		}
		result2 = result2 + "}:"+ to_string(currSup);
		frequentSets.insert(result2);
	} else {
		--pos;
		generateSubsets_r(result,  pos, max_support, frequent);
		
		size_t minSupport = currSup < max_support? currSup : max_support;
		
		generateSubsets_r(result2, pos, minSupport, frequent);
	}
}

void generateSubsets(vector< Tuple > & frequent){
	generateSubsets_r("", frequent.size()-1, UINT_MAX, frequent);
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
		std::cout << std::string(depth, ' ') << id << ": "<<count << endl;
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
	
	
	//ASSUMPTION - already checked this tree was a list. Never on branching trees.
	void createFrequentPatterns(){
		Node * temp = root;
		
		vector< Tuple > frequent;
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
					Tuple itemSupport;
					itemSupport.first = child.first;
					itemSupport.second = child.second->count;
					frequent.push_back(itemSupport);
				}
			}
		}
		
		if (frequent.size() > 0){
			generateSubsets(frequent);
		}
	}
	
	void conditional(){
		if(instances.size() > 0){
			for (auto itemNodeSet : instances){
				Tree temp;
				
				for(auto itemNode : itemNodeSet.second){
					list<int> pathUp;
					int count = itemNode->climb(pathUp);
					temp.insert(pathUp, count);
				}
				
				if (temp.isList()){
					temp.createFrequentPatterns();
				} else {
					temp.conditional();
				}
			}
		}
		
	}
};

bool comparePairVal (Tuple a, Tuple b){
	return a.first > b.first;
}

bool comparePairFreq (Tuple a, Tuple b){
	return a.second < b.second;
}

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
		vector<Tuple> pairs;
		
		for (auto v = transaction.begin(); v != transaction.end(); v++){
			//Create a list of <value, frequency> from transaction.
			auto f= frequency.find(*v);
			if (f != frequency.end()){
				Tuple x;
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

void testGenerateSubsets(){
	Tuple a;
	a.first = 193;
	a.second = 1;
	
	Tuple b;
	b.first = 904;
	b.second = 2;
	
	Tuple c;
	c.first = 909;
	c.second = 3;
	
	Tuple d;
	d.first = 248;
	d.second = 4;
	
	vector< Tuple > frequent;
	frequent.push_back(a);
	frequent.push_back(b);
	frequent.push_back(c);
	frequent.push_back(d);
	generateSubsets(frequent);
}

void saveResults(string outputFile){
	ofstream output;
	output.open(outputFile, ofstream::out);
	
	output <<"Number of frequent patterns found : "<<frequentSets.size()<<endl;
	
	for (string s : frequentSets){
		output << s << endl;
	}
	
	output.close();
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
	
	cout <<endl << "Transaction File       : " << dataFile << endl;
	cout        << "Output File            : "<<outputFile<<endl;
	numTransactions = getNumTransactions();
	
	minsup = numTransactions * minsup_ratio;
	
	cout <<"Number of transactions : "<<numTransactions<<endl;
	cout <<"Minimum support        : "<<minsup<<endl;
	
	fpgrowth();
	
	saveResults(outputFile);
	
	cout <<endl <<"All processing complete"<<endl;
	return 0;
}
