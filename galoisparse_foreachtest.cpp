
#include <iostream>
#include <unordered_map>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <Galois/Galois.h>
#include <Galois/Graph/Graph.h>
#include "Galois/Statistic.h"
//#include <cstring>
#include <string>
#include <fstream>
#include "Galois/Runtime/ll/PaddedLock.h"
using namespace std;
using namespace boost;

#define FOREACHTEST 0

typedef enum{pi,po,nd,op} node_type;
typedef Galois::Runtime::LL::PaddedLock<true> Lock_ty;

Lock_ty mutex;

struct Node {
 string label_type;//a0,b0...
 node_type type;// pi,po,nd
 bool simplified;
 //Lock_ty& mutex;
 int level;
 };

typedef Galois::Graph::FirstGraph<Node,int,true> Graph;

Graph g;

int node_index = 0;
vector<Graph::GraphNode> gnodes;

vector<string> fields;

bool checkWorkability(Graph::GraphNode gnode){
	int count = 0;
	for (Graph::edge_iterator edge : g.out_edges(gnode)) {
		int edgedata = g.getEdgeData(edge);
	   	if(edgedata == 1|| edgedata ==2)
	   		count++;
	   	}
	if (count > 1)
		return false;
	else
		return true;
}

class CompareNodelevel{
	
public:
			bool operator()(Graph::GraphNode& src1, Graph::GraphNode& src2)
			{
				if(g.getData(src2).level <= g.getData(src1).level)
					return true;
					
				return false;
			}
};

bool isEqualNodes(Graph::GraphNode gnode1, Graph::GraphNode gnode2){
	int count = 0;
	
	if(g.getData(gnode1).label_type==g.getData(gnode2).label_type){
		//cout<< "same node"<<endl;
		return true;
	}

	for (Graph::edge_iterator edge1 : g.out_edges(gnode1)){
		for (Graph::edge_iterator edge2 : g.out_edges(gnode2)){
			if(g.getEdgeData(edge1)== 3)
				if(g.getEdgeData(edge2)== 3)
					if(g.getEdgeDst(edge1)==g.getEdgeDst(edge2)){
						count++;
					}
		}
	}
	if(count == 2)
		return true;
	else
		return false;
}

bool find_cut(Graph::GraphNode &top,Graph::GraphNode &child_left,Graph::GraphNode &child_right)
{
	if ( g.getData(top).level != 0 && g.getData(top).level != 1) {
		for (Graph::edge_iterator edge : g.out_edges(top)){
			Graph::GraphNode dst = g.getEdgeDst(edge);
			if(g.getEdgeData(edge) == 3){	
				if(child_left==NULL){
					child_left=dst;
				}	
				else{
					child_right=dst;
				}
			}
		}
		return true;			
	}
	else 
		return false;
}
void parseInput(unordered_map <string, int> &map, unsigned i){
	Node n;
	for(i;i<fields.size();i++){
		if(fields[i].size()>1){
			n.type = pi;
			n.level = 0;
			n.label_type = fields[i].substr(0,fields[i].length()-1);
			//remove last char b/c its ',' or ';'
			map[fields[i].substr(0,fields[i].length()-1)] = node_index;
			//gnodes[node_index] = g.createNode(nodes[node_index]);
			gnodes.push_back(g.createNode(n));

			g.addNode(gnodes[gnodes.size()-1]);
			//cout << "node index: "<< node_index<<endl;
			node_index++;
		}

	}
}
void parseOutput(unordered_map <string, int> &map, unsigned i){
	Node n, output;
	for(i ;i<fields.size();i++){
		if(fields[i].size()>1){
			n.type = po;
			n.level = -1;
			n.label_type = fields[i].substr(0,fields[i].length()-1);

			output.type = op;
			output.level = -2;
			output.label_type = n.label_type+"_out";
			//remove last char b/c its ',' or ';'
			map[fields[i].substr(0,fields[i].length()-1)] = node_index;
			//gnodes[node_index] = g.createNode(nodes[node_index]);
			gnodes.push_back(g.createNode(n));
			g.addNode(gnodes[gnodes.size()-1]);
			//cout << "node index: "<< node_index<<endl;
			node_index++;

			map[output.label_type] = node_index;
			gnodes.push_back(g.createNode(output));
			g.addNode(gnodes[gnodes.size()-1]);

			g.getEdgeData(g.addEdge(gnodes[node_index-1],gnodes[node_index])) = 1;
			node_index++;
		}

	}
}

void parseWire(unordered_map <string, int> &map, unsigned i){
	Node n;
	for(i;i<fields.size();i++){
		if(fields[i].size()>1){
			n.type = nd;
			n.level = -2;
			n.label_type = fields[i].substr(0,fields[i].length()-1);
			//remove last char b/c its ',' or ';'
			map[fields[i].substr(0,fields[i].length()-1)] = node_index;
			//gnodes[node_index] = g.createNode(nodes[node_index]);
			gnodes.push_back(g.createNode(n));

			g.addNode(gnodes[gnodes.size()-1]);
			//cout << "node index: "<< node_index<<endl;
			node_index++;
		}

	}
}
void parseAssign(unordered_map <string, int> &map){
	int level1,level2;
	string f1,f3,f5;
	f1 = fields[1];
	f3 = fields[3];
	f5 = fields[5];
	if(fields[4].compare("|") == 0)
	{	//Edge weight: 2- Negative 1 - Positive
		f3 = f3.substr(0,f3.length());
		f5 = f5.substr(0,f5.length()-1);
		size_t invertf3 = f3.find_first_of('~');
		size_t invertf5 = f5.find_first_of('~');
		if(invertf3 != string::npos){
			f3 = f3.substr(1,f3.length());

			g.getEdgeData(g.addEdge(gnodes[map[f3]]//src
									   ,gnodes[map[f1]])) = 1;//dest
		}
		else
			g.getEdgeData(g.addEdge(gnodes[map[f3]]//src
												   ,gnodes[map[f1]])) = 2;//dest
		if(invertf5 != string::npos){
			f5 = f5.substr(1,f5.length());
			g.getEdgeData(g.addEdge(gnodes[map[f5]]
									,gnodes[map[f1]])) = 1;
		}

		else
			g.getEdgeData(g.addEdge(gnodes[map[f5]]
									   ,gnodes[map[f1]])) = 2;

		//cout << fields[1] << fields[3] << fields[5] << endl;
		g.getEdgeData(g.addEdge(gnodes[map[f1]],gnodes[map[f1+"_out"]])) = 2; //****Hack*** Use edge iterators for identifying final nodes

		level1 = g.getData(gnodes[map[f3]]).level;
		level2 = g.getData(gnodes[map[f5]]).level;
		g.getEdgeData(g.addEdge(gnodes[map[f1]],
				  gnodes[map[f3]])) = 3;//back edges to determine fanins -> wt. = 3
		g.getEdgeData(g.addEdge(gnodes[map[f1]],
				  gnodes[map[f5]])) = 3;//back edges to determins fanins -> wt. = 3
	}

	else
	{	size_t invert = fields[3].find_first_of("~");
		if (invert != string::npos){
			f3 = f3.substr(1,f3.length());
			//addEdge(src,dest)
			//remove the '~' from the start of string
			//map[string] returns node index to find which node
			g.getEdgeData(g.addEdge(gnodes[map[f3]],gnodes[map[f1]])) = 2;//
			level1 = g.getData(gnodes[map[f3]]).level;
			g.getEdgeData(g.addEdge(gnodes[map[f1]],
						  gnodes[map[f3]])) = 3;// Back edges to determine fanins -> wt. = 3
		}
		else{
			g.getEdgeData(g.addEdge(gnodes[map[f3]],gnodes[map[f1]])) = 1;
			level1 = g.getData(gnodes[map[f3]]).level;
			g.getEdgeData(g.addEdge(gnodes[map[f1]],
					  gnodes[map[f3]])) = 3;// Back edges to determine fanins -> wt. = 3

		}
		invert = fields[5].find_first_of("~");
		if (invert != string::npos){
			f5 = f5.substr(1,f5.length()-2);
			//remove first and last char b/c '~' to start and ';' at th
			//cout <<fields[5] << " substr"<<fields[5].substr(1,fields[5].length()-1)<<" "<<map[fields[5].substr(1,fields[5].length()-1)]<< " "<< nodes[map[fields[5].substr(1,fields[5].length()-1)]].label_type <<endl;
			g.getEdgeData(g.addEdge(gnodes[map[f5]],gnodes[map[f1]])) = 2;
			level2 = g.getData(gnodes[map[f5]]).level;
			g.getEdgeData(g.addEdge(gnodes[map[f1]],
					  gnodes[map[f5]])) = 3;// Back edges to determine fanins -> wt. = 3
		}
		else{
			f5 = f5.substr(0,f5.length()-1);
			g.getEdgeData(g.addEdge(gnodes[map[f5]],gnodes[map[f1]])) = 1;
			level2 = g.getData(gnodes[map[f5]]).level;
			g.getEdgeData(g.addEdge(gnodes[map[f1]],
					  gnodes[map[f5]])) = 3;// Back edges to determine fanins -> wt. = 3
		}
	}

	if(level1>=level2)
		g.getData(gnodes[map[fields[1]]]).level= level1+1;
	else
		g.getData(gnodes[map[fields[1]]]).level= level2+1;
}

void parseFileintoGraph(string inFile, unordered_map <string, int> &map){
	ifstream fin;
	string previous = "";
	//Node output;
	fin.open(inFile); // open a file
	if (!fin.good()){
		cout << "file not found\n";

	}
	else
	// read each line of the file
	while (!fin.eof())
		{
		// read an entire line into memory
		string line;
		getline(fin,line);
		trim(line);
		split(fields, line, is_any_of(" "));
		Node n;

		if(fields[0].compare("input") == 0){
			parseInput(map, 1);
			previous = "input";
		}
		else if(fields[0].compare("output") == 0){
			parseOutput(map, 1);
		}
		else if(fields[0].compare("wire") == 0){
			parseWire(map, 1);
			previous = "wire";
		}
		else if(fields[0].compare("assign") == 0){
			parseAssign(map);
			previous = "assign";
		}
		/*multiple lines gets 2nd line
		 * eg Input a b c d
		 * 			e f g h
		 */
		else if(previous.compare("input")==0){
			parseInput(map, 0);
		}
		else if(previous.compare("output")==0){
			parseOutput(map, 0);
		}
		else if(previous.compare("wire")==0){
			parseWire(map, 0);
		}
		/*
		if(fields[0].compare("input") == 0 || fields[0].compare("output") == 0||fields[0].compare("wire") == 0)
		{

			/*
			for(unsigned i =1;i<fields.size();i++){
				//add to hash map to index to corresponding node in array
				if(fields[i].size()>1)
				{
					//Add data to each node for level and type of input
					if(fields[0].compare("input") == 0)
					{
						n.type = pi;
						n.level = 0;
						n.label_type = fields[i].substr(0,fields[i].length()-1);

					}
					if(fields[0].compare("output") == 0)
					{
						n.type = po;
						n.level = -1;
						n.label_type = fields[i].substr(0,fields[i].length()-1);


						output.type = op;
						output.level = -2;
						output.label_type = n.label_type+"_out";
					
					}
					if(fields[0].compare("wire") == 0)
					{
						n.type = nd;
						n.level = -2;
						n.label_type = fields[i].substr(0,fields[i].length()-1);
					}
					//cout<<"Check ----> "<< fields[i].substr(0,fields[i].length()-1) <<endl;
					//remove last char b/c its ',' or ';'
					map[fields[i].substr(0,fields[i].length()-1)] = node_index;
					//gnodes[node_index] = g.createNode(nodes[node_index]);
					gnodes.push_back(g.createNode(n));

					g.addNode(gnodes[gnodes.size()-1]);
					//cout << "node index: "<< node_index<<endl;
					node_index++;
					//For storing output from the output node(No dangling edges on the final node) in the case that it is negated like in OR case
					if(fields[0].compare("output") == 0)
					{
						map[output.label_type] = node_index;
						gnodes.push_back(g.createNode(output));
						g.addNode(gnodes[gnodes.size()-1]);

						g.getEdgeData(g.addEdge(gnodes[node_index-1],gnodes[node_index])) = 1;
						node_index++;
					}
				}

			}

		}
		*/
		//cout <<"compare assign"<<endl;
		/*
		if(fields[0].compare("assign") == 0)
		{
			int level1,level2;
			string f1,f3,f5;
			f1 = fields[1];
			f3 = fields[3];
			f5 = fields[5];
			if(fields[4].compare("|") == 0)
			{	//Edge weight: 2- Negative 1 - Positive
				f3 = f3.substr(0,f3.length());
				f5 = f5.substr(0,f5.length()-1);
				g.getEdgeData(g.addEdge(gnodes[map[f3]]//src
											   ,gnodes[map[f1]])) = 2;//dest

				g.getEdgeData(g.addEdge(gnodes[map[f5]]
											   ,gnodes[map[f1]])) = 2;

				//cout << fields[1] << fields[3] << fields[5] << endl;
				g.getEdgeData(g.addEdge(gnodes[map[f1]],gnodes[map[f1+"_out"]])) = 2; //****Hack*** Use edge iterators for identifying final nodes
	
				level1 = g.getData(gnodes[map[f3]]).level;
				level2 = g.getData(gnodes[map[f5]]).level;
				g.getEdgeData(g.addEdge(gnodes[map[f1]],
						  gnodes[map[f3]])) = 3;//back edges to determine fanins -> wt. = 3
				g.getEdgeData(g.addEdge(gnodes[map[f1]],
						  gnodes[map[f5]])) = 3;//back edges to determins fanins -> wt. = 3
			}

			else
			{	size_t invert = fields[3].find_first_of("~");
				if (invert != string::npos){
					f3 = f3.substr(1,f3.length());
					//addEdge(src,dest)
					//remove the '~' from the start of string
					//map[string] returns node index to find which node
					g.getEdgeData(g.addEdge(gnodes[map[f3]],gnodes[map[f1]])) = 2;//
					level1 = g.getData(gnodes[map[f3]]).level;
					g.getEdgeData(g.addEdge(gnodes[map[f1]],
								  gnodes[map[f3]])) = 3;// Back edges to determine fanins -> wt. = 3
				}
				else{
					g.getEdgeData(g.addEdge(gnodes[map[f3]],gnodes[map[f1]])) = 1;
					level1 = g.getData(gnodes[map[f3]]).level;
					g.getEdgeData(g.addEdge(gnodes[map[f1]],
							  gnodes[map[f3]])) = 3;// Back edges to determine fanins -> wt. = 3

				}
				invert = fields[5].find_first_of("~");
				if (invert != string::npos){
					f5 = f5.substr(1,f5.length()-2);
					//remove first and last char b/c '~' to start and ';' at th
					//cout <<fields[5] << " substr"<<fields[5].substr(1,fields[5].length()-1)<<" "<<map[fields[5].substr(1,fields[5].length()-1)]<< " "<< nodes[map[fields[5].substr(1,fields[5].length()-1)]].label_type <<endl;
					g.getEdgeData(g.addEdge(gnodes[map[f5]],gnodes[map[f1]])) = 2;
					level2 = g.getData(gnodes[map[f5]]).level;
					g.getEdgeData(g.addEdge(gnodes[map[f1]],
							  gnodes[map[f5]])) = 3;// Back edges to determine fanins -> wt. = 3
				}
				else{
					f5 = f5.substr(0,f5.length()-1);
					g.getEdgeData(g.addEdge(gnodes[map[f5]],gnodes[map[f1]])) = 1;
					level2 = g.getData(gnodes[map[f5]]).level;
					g.getEdgeData(g.addEdge(gnodes[map[f1]],
							  gnodes[map[f5]])) = 3;// Back edges to determine fanins -> wt. = 3
				}
			}

		if(level1>=level2)
			g.getData(gnodes[map[fields[1]]]).level= level1+1;
		else
			g.getData(gnodes[map[fields[1]]]).level= level2+1;
		}
		*/

	}
}

void getChildren(Graph::GraphNode parent, Graph::GraphNode &child1, Graph::GraphNode &child2){
	bool  a = true;
	for (Graph::edge_iterator edge : g.out_edges(parent)){
			if(g.getEdgeData(edge)==3){
				if (a){
					child1 = g.getEdgeDst(edge);
					a = !a;
				}
				else
					child2 = g.getEdgeDst(edge);
			}
		}
}

void make_replacement(Graph::GraphNode node, Graph::GraphNode match_node){
	Graph::GraphNode child1,child2,temp;
	getChildren(node,child1,child2);
	for(Graph::edge_iterator edge : g.out_edges(match_node)){
		temp = g.getEdgeDst(edge);
		if(!((isEqualNodes(child1,temp))||isEqualNodes(child2,temp))){
			g.getEdgeData(g.addEdge(node,temp))=g.getEdgeData(edge);
			g.getEdgeData(g.addEdge(temp,node))=3;//Assigning back edges from the parent nodes of the removed copy node to the original node
		}
	}
	g.removeEdge(child1,g.findEdge(child1,match_node));
	g.removeEdge(child2,g.findEdge(child2,match_node));
	g.removeNode(match_node);
}

bool checkxor(Graph::GraphNode node){
	Graph::GraphNode inode1=NULL,inode2=NULL;
	Graph::GraphNode input1=NULL,input2=NULL,input3=NULL,input4=NULL;
	getChildren(node,inode1,inode2);
	//cout<<g.getData(inode1).label_type<<" "<<g.getData(inode2).label_type<<endl;
	getChildren(inode1, input1, input2);
	getChildren(inode2, input3, input4);
/*
	cout << "got children\n";
	cout <<"node "<< g.getData(node).label_type << "\n";
	cout <<"child1 "<< g.getData(inode1).label_type << "\n";
	cout <<"child2 "<< g.getData(inode2).label_type << "\n";
*/
	if(g.getEdgeData(g.findEdge(inode1,node))==2 && g.getEdgeData(g.findEdge(inode2,node))==2){
	if(isEqualNodes(input1,input3)&&isEqualNodes(input2,input4))
		if(g.getEdgeData(g.findEdge(input1,inode1))!=g.getEdgeData(g.findEdge(input3,inode2))){
			//cout << "nequal edges\n";
			if(g.getEdgeData(g.findEdge(input2,inode1))!=g.getEdgeData(g.findEdge(input4,inode2))){
				//cout << "true\n";
				return true;
			}
		}
	}
	//cout<<"false\n";
	return false;
}
/*
bool checkxnor(Graph::GraphNode node){
	find_cut(node)


}
*/

void convertxor_cost(Graph::GraphNode node){
	int cost=0;
	Graph::GraphNode inode1=NULL,inode2=NULL;
	Graph::GraphNode input1=NULL,input2=NULL,input3=NULL,input4=NULL;
	string child_label,parent_label;
	getChildren(node,inode1,inode2);
	getChildren(inode1, input1, input2);
	getChildren(inode2, input3, input4);

	if(g.getEdgeData(g.findEdge(input1,inode1))==2){
		//cout<<"Reassigning edge for "<<g.getData(input1).label_type<<g.getData(inode1).label_type<<endl;
		g.getEdgeData(g.addEdge(input1,inode1))=1;
		//cout<<"Changed edge data "<<g.getEdgeData(g.findEdge(input1,inode1))<<endl;		
	}
	else
		g.getEdgeData(g.addEdge(input2,inode1))=1;

	if(g.getEdgeData(g.findEdge(input3,inode2))==1)
		g.getEdgeData(g.addEdge(input3,inode2))=2;
	else
		g.getEdgeData(g.addEdge(input4,inode2))=2;

	for (Graph::edge_iterator edge : g.out_edges(node)){
		//cout<<"Node: "<<g.getData(node).label_type<<" -> "<<g.getData(g.getEdgeDst(edge)).label_type<<"Weight ="<< g.getEdgeData(edge)<<endl;

		if(g.getEdgeData(edge) == 2)
			g.getEdgeData(edge) = 1;
		else if(g.getEdgeData(edge) == 1)
			g.getEdgeData(edge) = 2;

	}
	/*Compute cost*/
	for (Graph::edge_iterator edge : g.out_edges(input1)){
		Graph::GraphNode dst = g.getEdgeDst(edge);
		if((g.getData(dst).label_type != g.getData(inode1).label_type) &&  (g.getData(dst).label_type != g.getData(inode2).label_type)){
			if(isEqualNodes(dst,inode1)){
				//cout<<"Match found: "<< g.getData(dst).label_type <<endl;
				make_replacement(inode1,dst);
				cost++;
			}
			else if(isEqualNodes(dst,inode2)){
				//cout<<"Match found: "<< g.getData(dst).label_type <<endl;
				make_replacement(inode2,dst);
				cost++;
			}
		}
	}

}

#ifdef FOREACHTEST
struct Process { 
	void operator()(Graph::GraphNode src, Galois::UserContext<Graph::GraphNode>& ctx) {  
		/*if(g.getData(src).label_type=="n15"){
			sleep(10);
			ctx.breakLoop();
		}*/
		/* alex change
		mutex.lock();
		//cout<<"Node:"<<g.getData(src).label_type<<" "<<"Level:"<<g.getData(src).level<<endl;
		if(checkxor(src)){
			convertxor_cost(src);
		}
		mutex.unlock();
		//ctx.breakLoop();
		 *
		 */
		if(checkxor(src)){
			mutex.lock();
			convertxor_cost(src);
			mutex.unlock();
		}
	}
};
#else
struct Process { 
	void operator()(Graph::GraphNode src) { 
		cout<<"Inside proess from do_all"<<endl; 
		/*if(g.getData(src).label_type=="n15"){
			sleep(10);
			ctx.breakLoop();
		}*/
		
		//cout<<"Node:"<<g.getData(src).label_type<<" "<<"Level:"<<g.getData(src).level<<endl;
		if(checkxor(src)){
			//convertxor_cost(src);
		}
		//ctx.breakLoop();
	}
};
#endif

int main(int argc, char *argv[]) {
	Galois::StatManager statManager;
	Galois::StatTimer T("total");
	unordered_map <string, int> map;
	//Lock_ty mutex;
	priority_queue<Graph::GraphNode,vector<Graph::GraphNode>, CompareNodelevel> pq;
	if ( argc != 3 ){
    	cout<<"usage: "<< argv[0] <<" <filename> <Num of Threads>\n";
		return 0;
	}
 
  	parseFileintoGraph(argv[1],map);
	unsigned int numThreads = atoi(argv[2]);
	numThreads = Galois::setActiveThreads(numThreads);
	//cout << g.getData(gnodes[0]).label_type << endl;
	// Traverse graph

	//print graph first time
	for ( Graph::GraphNode src : g){
		cout <<"src: "<< g.getData(src).label_type;
		cout <<" level: "<<g.getData(src).level;

		//g.getData(src).mutex = mutex;
		if(g.getData(src).level > 1)
			pq.push(src);
		
		for (Graph::edge_iterator edge : g.out_edges(src)) {
   			Graph::GraphNode dst = g.getEdgeDst(edge);
     		cout <<" dest: "<< g.getData(dst).label_type;
     		int edgeData = g.getEdgeData(edge);
     		cout << " edge data " << edgeData;
   		}
	   	cout <<endl;
 	}

	//prints what? nodes being refactored?
	//does what?
	int level=2;

	vector<Graph::GraphNode> temp;
	while (!pq.empty()) {
		cout << "level: " <<level<<'\n';
		cout<<"Node inside level "<<level<<": "<<g.getData(pq.top()).label_type<<endl;
		while(g.getData(pq.top()).level==level && !pq.empty()){

			 //cout<<"Node inside level while 2 loop "<<level<<":"<<g.getData(pq.top()).label_type<<endl;
			 temp.push_back(pq.top());
			 pq.pop();
			 if(pq.empty())
				break;
			 cout << "next node: " << g.getData(pq.top()).label_type <<'\n';
			 cout << "next node level: " << g.getData(pq.top()).level <<'\n';
		}
		cout << "size: " << temp.size() << '\n';
		cout << "end node: "<<g.getData(temp[temp.size()-1]).label_type<<'\n';
		#ifdef FOREACHTEST
			Galois::for_each(temp.begin(),temp.end(),Process());
		#else
			Galois::do_all(temp.begin(),temp.end(), Process(),Galois::loopname("Tyler"));
		#endif
			cout << "after process\n";
			temp.erase(temp.begin(),temp.end());
			//cout<<"Before level increment:"<<g.getData(pq.top()).level<<endl;
			level++;
    }

	cout << "after changes\n";
	//print graph after refactoring of nodes?
	for ( Graph::GraphNode src : g){

		if(g.getData(src).type == po){
			cout <<"src: "<< g.getData(src).label_type;
			cout <<" level: "<<g.getData(src).level;

			for (Graph::edge_iterator edge : g.out_edges(src)) {
				Graph::GraphNode dst = g.getEdgeDst(edge);
				cout <<" dest: "<< g.getData(dst).label_type;
				int edgeData = g.getEdgeData(edge);
				cout << " edge data " << edgeData;
				//assert(edgeData == 5);
			}
			cout <<endl;
		}

 	}

	return 0;
}
