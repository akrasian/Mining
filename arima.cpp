#include <iostream>
#include <map>
#include <set>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>

using namespace std;

typedef set< int > ItemSet;
typedef map< ItemSet, int > SuperSet;

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


//Returns array of all strings in s containing none of the chars in tokens.
vector<string> split (const string& s, string tokens){
	vector<string> res;
	
	string curr = "";
	for (char c : s){
		bool istoken = false;
		for (char d : tokens){
			if (d == c)
				istoken = true;
		}
		if (!istoken){
			curr += c;
		} else {
			if (curr.length() > 0){
				res.push_back(curr);
				curr = "";
			}
		}
	}
	
	if (curr.length() > 0){
		res.push_back(curr);
		curr = "";
	}
	return res;
}

SuperSet parseSuperSet(string line){
	SuperSet result;
	
	vector<string> itemSupportSets = split(line, "()");
	for (auto x: itemSupportSets){
		vector<string> itemsetVector = split(x, "{},:");
		
		ItemSet curr;
		
		for(int i = 0; i<itemsetVector.size()-1; ++i){
			int val;
			string item = itemsetVector[i];
			istringstream parser(item);
			parser >> val;
			curr.insert(val);
		}
		
		int support;
		string item = itemsetVector[itemsetVector.size()-1];
		istringstream parser(item);
		parser >> support;
		
		result[curr] = support;
		
	}
	
	return result;
}

int parseRareFile(string rareFile, SuperSet & rareGenerators){
	ifstream input;
	input.open(rareFile, ifstream::in);
	string line;
	
	int linenum = 0;
	while (getline(input, line)){
		if (linenum %5 == 1){
			cout <<"Frequent"<<endl;
			cout << line << endl;
			SuperSet x = parseSuperSet(line);
			printSuperSet(x, std::cout);
		} else if (linenum % 5 == 3){
			//~ cout <<"Rare"<<endl;
			//~ 
			//~ cout << line << endl;
		}
		linenum++;
	}
	
	input.close();
	//~ return num;
	return 0;
}


int main(){
	SuperSet rare;
	parseRareFile("output.txt", rare);
	
	return 0;
}
