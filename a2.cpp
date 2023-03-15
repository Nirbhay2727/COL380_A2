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
    ofstream f1("./test0/our_input.txt");
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




int main(int argc, char* argv[]) {

    //input options
    int option;
    int taskid = 0;
    string inputpath="./test0/test-input-0.gra";
    string headerpath="";
    string outputpath="./test0/our_output.txt";
    int verbose=0;
    int startk=1;
    int endk=10;
    int p;
    std::vector<std::string> args(argv, argv + argc);
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == "--taskid" && i + 1 < args.size()) {
                taskid = std::stoi(args[i + 1]);
        }
        if (args[i] == "--inputpath" && i + 1 < args.size()) {
                inputpath = args[i + 1];
        }
        if (args[i] == "--headerpath" && i + 1 < args.size()) {
                headerpath = args[i + 1];
        }
        if (args[i] == "--outputpath" && i + 1 < args.size()) {
                outputpath = args[i + 1];
        }
        if (args[i] == "--verbose" && i + 1 < args.size()) {
                verbose = std::stoi(args[i + 1]);
        }
        if (args[i] == "--startk" && i + 1 < args.size()) {
                startk = std::stoi(args[i + 1]);
        }
        if (args[i] == "--endk" && i + 1 < args.size()) {
                endk = std::stoi(args[i + 1]);
        }
    }



    //start
    int n,m;
    map<int,set<int> > adjList;
    readInputFromFile(inputpath,n,m,adjList);
    map<pair<int,int>,int> supp;
    std::ofstream outfile(outputpath);

    for(int k = startk; k <= endk; k++){

        //prefilter
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
        }

        map<pair<int,int>,int> supp;
        
        //initialize
        set<pair<int,int> > deletable2;
        for (auto tempset:adjList){
            int a = tempset.first;
            for (auto b:tempset.second){
                set<int> Intersection;
                insert_iterator<set<int> > IntersectIterate(Intersection, Intersection.begin());
                set_intersection(adjList[a].begin(), adjList[a].end(), adjList[b].begin(), adjList[b].end(), IntersectIterate);
                int supp_e=Intersection.size();
                if(a<b)
                    supp[{a,b}] = supp_e;
                if (supp_e<k - 2 && a<b){
                    deletable2.insert({a,b});
                }
            }
        }


        //filter edges
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
            insert_iterator<set<int> > IntersectIterate(Intersection, Intersection.begin());
            set_intersection(adjList[a].begin(), adjList[a].end(), adjList[b].begin(), adjList[b].end(), IntersectIterate);
            for(auto c:Intersection){
                if(a<c){
                    supp[{a,c}]--;
                    if(supp[{a,c}]<k-2){
                        deletable2.insert({a,c});
                    }
                }else{
                    supp[{c,a}]--;
                    if(supp[{c,a}]<k-2){
                        deletable2.insert({c,a});
                    }
                }

                if(b<c){
                    supp[{b,c}]--;
                    if(supp[{b,c}]<k-2){
                        deletable2.insert({b,c});
                    }
                }else{
                    supp[{c,b}]--;
                    if(supp[{c,b}]<k-2){
                        deletable2.insert({c,b});
                    }
                }

                // set<int> tempIntersection;
                // insert_iterator<set<int>> IntersectIterate2(tempIntersection, tempIntersection.begin());
                // set_intersection(adjList[a].begin(), adjList[a].end(), adjList[c].begin(), adjList[c].end(), IntersectIterate2);
                // int supp=tempIntersection.size();
                // if (supp<k - 2){
                //     a<c?deletable2.insert(make_pair(a,c)):deletable2.insert(make_pair(c,a));
                // }
                // tempIntersection.clear();

                // insert_iterator<set<int>> IntersectIterate3(tempIntersection, tempIntersection.begin());
                // set_intersection(adjList[b].begin(), adjList[b].end(), adjList[c].begin(), adjList[c].end(), IntersectIterate3);
                // supp=tempIntersection.size();
                // if (supp<k - 2){
                //     b<c?deletable2.insert(make_pair(b,c)):deletable2.insert(make_pair(c,b));
                // }
            }
        }

        //output
        if(adjList.size()!=0){
            outfile << "1" << endl;
            if(verbose==1){
                for(auto e:adjList){
                    outfile<<e.first<<" ";
                }
            outfile<<endl;
            }
        }
        else{
            outfile<<"0"<<endl;
        }
    }
    return 0;
}