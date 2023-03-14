#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <bits/stdc++.h>
#include <map>
using namespace std;

void readInputFromFile(const string& filename, int& n, int& m, vector<vector<int>>& adjList,vector<pair<int, int>>& edges) {
    ifstream infile(filename, ios::binary); 
    if (!infile.is_open()) { 
        cerr << "Error: Failed to open file \"" << filename << "\"" << endl;
        exit(1); 
    }

    infile.read(reinterpret_cast<char*>(&n), sizeof(n)); 
    n = __builtin_bswap32(n); 
    adjList.resize(n); 

    infile.read(reinterpret_cast<char*>(&m), sizeof(m)); 
    m = __builtin_bswap32(m); 


    for (int i = 0; i < n; i++) {
        int node,k;
        infile.read(reinterpret_cast<char*>(&node), sizeof(node)); 
        node = __builtin_bswap32(node); 
        infile.read(reinterpret_cast<char*>(&k), sizeof(k)); 
        k = __builtin_bswap32(k); 
        adjList[node].resize(n); 
        for (int j = 0; j < k; j++) {
            int temp;
            infile.read(reinterpret_cast<char*>(&temp), sizeof(temp)); 
            temp = __builtin_bswap32(temp); 
            adjList[node][temp]=1; 
            edges.push_back(make_pair(temp,i));
        }
    }
    infile.close(); 
}


void calculateSupp(map<pair<int,int>,int>& supp, vector<pair<int, int>>& edges,int n,vector<vector<int>>& adj,int& kmin,int& kmax){
    int min_supp=INT_MAX;
    int max_supp=INT_MIN;
    for (auto e : edges){
        int s = 0;
        for (int w = 0; w < n; w++) {
            int u = e.first;
            int v=e.second;
            if (w != u && w != v && adj[u][w] && adj[v][w]) {
                s++;
            }
        }
        supp[e]=s;
        min_supp=min(s,min_supp);
        max_supp=max(s,max_supp);
    }
    kmin=min_supp+2;   
    kmax=max_supp+2;
}


int main(int argc, char* argv[]) {
    vector<pair<int, int>> edges;
    int n,m;
    int kmin,kmax;
    string inputpath="";
    vector<vector<int>> adjList;
    readInputFromFile(inputpath,n,m,adjList,edges);
    map<pair<int,int>,int> supp;
    calculateSupp(supp,edges,n,adjList,kmin,kmax);
    map<pair<int,int>,int> tao;
    for(auto e:edges){
        tao[e]=supp[e]+2;
    }
    //truss computation
    pair<int,int> W=make_pair(kmin,kmin);
    

    return 0;
}