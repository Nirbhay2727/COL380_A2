#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <bits/stdc++.h>
#include <map>
#include <queue>
#include <algorithm>
#include <set>
using namespace std;

void readInputFromFile(const string& filename, int& n, int& m, map<int,set<int>>& adjList,vector<pair<int, int>>& edges) {
    ifstream infile(filename, ios::binary); 
    if (!infile.is_open()) { 
        cerr << "Error: Failed to open file \"" << filename << "\"" << endl;
        exit(1); 
    }

    infile.read(reinterpret_cast<char*>(&n), sizeof(n)); 
    n = __builtin_bswap32(n); 

    infile.read(reinterpret_cast<char*>(&m), sizeof(m)); 
    m = __builtin_bswap32(m); 


    for (int i = 0; i < n; i++) {
        int node,k;
        infile.read(reinterpret_cast<char*>(&node), sizeof(node)); 
        node = __builtin_bswap32(node); 
        infile.read(reinterpret_cast<char*>(&k), sizeof(k)); 
        k = __builtin_bswap32(k); 
        for (int j = 0; j < k; j++) {
            int temp;
            infile.read(reinterpret_cast<char*>(&temp), sizeof(temp)); 
            temp = __builtin_bswap32(temp); 
            adjList[node].insert(temp);
            if (node<temp){
                edges.push_back(make_pair(node,temp));
            }
        }
    }
    infile.close(); 
}




int main(int argc, char* argv[]) {
    vector<pair<int, int>> edges;
    int kmin = atoi(argv[1]);
    int kmax = atoi(argv[2]);
    int n,m;
    string inputpath="";
    map<int,set<int>> adjList;
    readInputFromFile(inputpath,n,m,adjList,edges);
    map<pair<int,int>,int> supp;
    int verbose;

    //prefilter
    for(int k = kmin; k <= kmax; k++){
        queue<int> deletable;
        for (auto it=adjList.begin();it!=adjList.end();it++){
            if (it->second.size()<k){
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
            for (auto it=adjList.begin();it!=adjList.end();it++){
                if(it->second.find(temp)!=it->second.end())
                    it->second.erase(temp);
            }
        }
        
        //initialize
        set<pair<int,int>> deletable2;
        for (auto tempset:adjList){
            int a = tempset.first;
            for (auto b:tempset.second){
                set<int> Intersection;
                insert_iterator<set<int>> IntersectIterate(Intersection, Intersection.begin());
                set_intersection(adjList[a].begin(), adjList[a].end(), adjList[b].begin(), adjList[b].end(), IntersectIterate);
                int supp=Intersection.size();
                if (supp<k - 2){
                    a<b?deletable2.insert(make_pair(a,b)):deletable2.insert(make_pair(b,a));
                }
            }
        }



        while(deletable2.size()>0){
            pair<int,int> temp = *deletable2.begin();
            deletable2.erase(temp);
            adjList[temp.first].erase(temp.second);
            if(adjList[temp.first].size()==0){
                adjList.erase(temp.first);
            }
            adjList[temp.second].erase(temp.first);
            if(adjList[temp.second].size()==0){
                adjList.erase(temp.second);
            }
            set<int> Intersection;
            int a = temp.first;
            int b = temp.second;
            insert_iterator<set<int>> IntersectIterate(Intersection, Intersection.begin());
            set_intersection(adjList[a].begin(), adjList[a].end(), adjList[b].begin(), adjList[b].end(), IntersectIterate);
            for(auto c:Intersection){
                set<int> tempIntersection;
                insert_iterator<set<int>> IntersectIterate2(tempIntersection, tempIntersection.begin());
                set_intersection(adjList[a].begin(), adjList[a].end(), adjList[c].begin(), adjList[c].end(), IntersectIterate2);
                int supp=tempIntersection.size();
                if (supp<k - 2){
                    a<c?deletable2.insert(make_pair(a,c)):deletable2.insert(make_pair(c,a));
                }
                tempIntersection.clear();

                insert_iterator<set<int>> IntersectIterate3(tempIntersection, tempIntersection.begin());
                set_intersection(adjList[b].begin(), adjList[b].end(), adjList[c].begin(), adjList[c].end(), IntersectIterate3);
                supp=tempIntersection.size();
                if (supp<k - 2){
                    b<c?deletable2.insert(make_pair(b,c)):deletable2.insert(make_pair(c,b));
                }
            }
        }
        if(verbose==1){

        }

        if(adjList.size()!=0){
            cout << k << "truss exists" << endl;
        }
    }

    return 0;
}