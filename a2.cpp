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




int main(int argc, char* argv[]) {

    //input options
    int option;
    int taskid = 0;
    string inputpath="./test1/test-input-1.gra";
    string headerpath="";
    string outputpath="./test1/our_output.txt";
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
        cout << k << " size=" << deletable2.size() << endl;
        for(auto temp:supp){
            // cout << temp.first.first << " " << temp.first.second << " " << temp.second << endl;
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
            //assertion a<b
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

            }
        }

        // if(k==8){
        //     for(auto temp:adjList){
        //         cout << temp.first << " =";
        //         for(auto temp2:temp.second){
        //             cout << temp2 << " ";
        //         }
        //         cout << endl;
        //     }
        // }

        //output
        if(adjList.size()!=0){
            bool flag = true;
            for(auto e:adjList){
                if(e.second.size()!=0){
                    flag = false;
                    break;
                }
            }
            if(!flag){
                outfile << "1" << endl;
                if(verbose==1){
                    for(auto e:adjList){
                        outfile<<e.first<<" ";
                    }
                outfile<<endl;
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