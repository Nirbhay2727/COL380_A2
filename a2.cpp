#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <bits/stdc++.h>
#include <map>
#include <queue>
#include <algorithm>
#include <set>
#include <unistd.h>
using namespace std;

void readInputFromFile(const string& filename, int& n, int& m, map<int,set<int> >& adjList) {
    ofstream f1("./test1/our_input.txt");
    ifstream infile(filename, ios::binary); 
    if (!infile.is_open()) { 
        cerr << "Error: Failed to open file \"" << filename << "\"" << endl;
        exit(1); 
    }

    infile.read(reinterpret_cast<char*>(&n), sizeof(n)); 

    infile.read(reinterpret_cast<char*>(&m), sizeof(m)); 
     

    f1<<n<<" "<<m<<endl;
    for (int i = 0; i < n; i++) {
        int node,k;
        infile.read(reinterpret_cast<char*>(&node), sizeof(node)); 
        infile.read(reinterpret_cast<char*>(&k), sizeof(k));
        f1<<node<<" "<<k<<" ";
        for (int j = 0; j < k; j++) {
            int temp;
            infile.read(reinterpret_cast<char*>(&temp), sizeof(temp)); 
            adjList[node].insert(temp);
            f1<<temp<<" ";
        }
        f1<<endl;
    }
    infile.close();
}

void dfs(map<int,set<int>>& G, vector<bool> &visited, int v, vector<int> &verts) {
    if(visited[v] == true) return;

    visited[v] = true;
    verts.push_back(v);

    for(int u: G[v]) {
        if(u!=v) {
            // cout<<u<<" ";
            dfs(G, visited, u, verts);
        }
    }
}

int main(int argc, char* argv[]) {

    //input options
    int taskid = -1;
    std::string inputpath;
    std::string headerpath;
    std::string outputpath;
    int verbose = -1;
    int startk = -1;
    int endk = -1;
    int p = -1;
    std::vector<std::string> args(argv, argv + argc);
    for (size_t i = 0; i < args.size(); i++) {
         std::string arg = argv[i];
        if (arg.substr(0, 10) == "--taskid=") {
        taskid = std::stoi(arg.substr(10));
        } else if (arg.substr(0, 12) == "--inputpath=") {
        inputpath = arg.substr(12);
        } else if (arg.substr(0, 12) == "--headerpath=") {
        headerpath = arg.substr(12);
        } else if (arg.substr(0, 12) == "--outputpath=") {
        outputpath = arg.substr(12);
        } else if (arg.substr(0, 10) == "--verbose=") {
        verbose = std::stoi(arg.substr(10));
        } else if (arg.substr(0, 9) == "--startk=") {
        startk = std::stoi(arg.substr(9));
        } else if (arg.substr(0, 7) == "--endk=") {
        endk = std::stoi(arg.substr(7));
        } else if (arg.substr(0, 4) == "--p=") {
        p = std::stoi(arg.substr(4));
        }
    }



    //start
    int n,m;
    map<int,set<int> > adjList;
    readInputFromFile(inputpath,n,m,adjList);
    map<pair<int,int>,int> supp;
    std::ofstream outfile(outputpath);

    for(int k = startk+2; k <= endk+2; k++){

        //prefilter
        queue<int> deletable;
        for (auto it=adjList.begin();it!=adjList.end();it++){
            if (it->second.size()<k-1){
                deletable.push(it->first);
            }
        }
        while(deletable.size()>0){
            int temp=deletable.front();
            deletable.pop();
            for (auto it=adjList[temp].begin();it!=adjList[temp].end();it++){
                adjList[*it].erase(temp);
                if (adjList[*it].size()<k - 1){
                    deletable.push(*it);
                }
            }
            adjList.erase(temp);
        }

        map<pair<int,int>,int> supp;
        
        //initialize
        set<pair<int,int> > deletable2;
        for (auto tempset:adjList){
            int a = tempset.first;
            for (auto b:tempset.second){
                if(a<b){
                    set<int> Intersection;
                    insert_iterator<set<int> > IntersectIterate(Intersection, Intersection.begin());
                    set_intersection(adjList[a].begin(), adjList[a].end(), adjList[b].begin(), adjList[b].end(), IntersectIterate);
                    int supp_e=Intersection.size();
                    supp[{a,b}] = supp_e;
                    // cout << a << " " << b << " " << supp_e << endl;
                    if (supp_e<k - 2){
                        deletable2.insert({a,b});
                    }
                }
            }
        }
        

        //output
        if(adjList.size()!=0){
            bool flag = true;
            auto it = adjList.begin();
            while(it!=adjList.end()){
                if(it->second.size()!=0){
                    flag = false;
                    it++;
                }else{
                    //delete it;
                    adjList.erase(it++);
                }
            }
            if(!flag){
                outfile << "1" << endl;
                if(verbose==1){
                    vector<vector<int>> components;
                    vector<bool> visited(n+1,0);
                    for(auto temp:adjList){
                        if(visited[temp.first]==0){
                            vector<int> component;
                            dfs(adjList,visited,temp.first,component);
                            components.push_back(component);
                        }
                    }
                    cout << "components size = " << components.size() << endl;
                    outfile<<components.size()<<endl;
                    for(auto component:components){
                        sort(component.begin(),component.end());
                        for(auto node:component){
                            outfile<<node<<" ";
                        }
                        outfile<<endl;
                    }
                }
            }else{
                outfile<<"0"<<endl;
            }
        }
        else{
            outfile<<"0"<<endl;
        }
    }
    return 0;
}