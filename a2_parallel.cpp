#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <set>
#include <unistd.h>
#include <unordered_set>
#include <mpi.h>


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


void filterEdges(map<int,set<int> >& G,map<pair<int,int>,int>& supp,int k,set<pair<int,int> >& deletable2){
    //filter edges
    set<pair<int, int>> my_edges;
    map<pair<int, int>, int> ownerp;
    map<pair<int, int>, bool> my_edge_map;

    int my_rank, num_procs;
    MPI_Comm_rank( MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size (MPI_COMM_WORLD, &num_procs);


    vector<pair<int, int>> edges;
    for(auto it=G.begin(); it!=G.end(); it++){
        for(auto it2=it->second.begin(); it2!=it->second.end(); it2++){
            if(it->first<*it2){
                edges.push_back(make_pair(it->first,*it2));
            }
        }
    }


    for(int it1 = my_rank; it1<edges.size(); it1+=num_procs) {
        my_edges.insert(edges[it1]);
        my_edge_map[edges[it1]] = true;
        // cout<<"edge "<<it1<<endl;
    }
    
    for(int i=0; i<edges.size(); i++){
        ownerp[edges[i]]=i%num_procs;
    }


    set<pair<int, int>> my_deletable;

    for(auto i : deletable2) {
        if(my_edges.find(i)!=my_edges.end()) {
            my_deletable.insert(i);
        }
    }

    for(auto i : my_deletable) {
        std::cout << i.first << " " << i.second << endl;
    }

    int sendcounts[num_procs];
    int sdispls[num_procs];
    int sedgedispls[num_procs];
    int sendedgecounts[num_procs];
    

    int recvcounts[num_procs];
    int rdispls[num_procs];
    int redgedispls[num_procs];
    int recvedgecounts[num_procs];

    for(int i = 0; i<num_procs; i++) {
        sendcounts[i]=3;
        recvcounts[i]=3;
        sdispls[i] = 3*i;
        rdispls[i] = 3*i;
    }

    bool all_fin = false;

    while(!all_fin){
        vector<int> recvbuf(3*num_procs);
        vector<int> sendbuf(3*num_procs);
        set<pair<pair<int, int>, int>> sendingedges[num_procs];

        for(int i=0; i<num_procs; i++)sendedgecounts[i]=0;

        int count = 0;
        if(my_deletable.empty() == false) {
           
            auto e = *my_deletable.begin();

            my_deletable.erase(e);
            my_edges.erase(e);
            my_edge_map[e] = false;
            
            int x1 = e.first, y1 = e.second;
            for(int a: G[x1]) {
                if(G[y1].find(a)!=G[y1].end()) {
                    
                    pair<int, int> ax={min(x1, a), max(x1, a)}, ay={min(y1, a), max(y1, a)};
                    #ifdef gg
                    if(e==make_pair(3, 11))cout<<"ax: "<<ax.first<<", "<<ax.second<<" ay: "<<ay.first<<", "<<ay.second<<" owner: "<<ownerp[ax]<<", "<<ownerp[ay]<<endl;
                    #endif
                    sendedgecounts[ownerp[ax]]+=3;
                    sendedgecounts[ownerp[ay]]+=3;
                    sendingedges[ownerp[ax]].insert({ax, y1});
                    sendingedges[ownerp[ay]].insert({ay, x1});

                }
            }
            
            for(int it1 = 0; it1<num_procs; it1++) {
                sendbuf[3*it1] = e.first;
                sendbuf[3*it1 + 1] = e.second;
                sendbuf[3*it1+2] = sendedgecounts[it1]/3;
            }
        } else {
            for(int it1 = 0; it1<num_procs; it1++) {
                sendbuf[3*it1] = -1;
                sendbuf[3*it1 + 1] = -1;
                sendbuf[3*it1+2] = 0;
            }
        }

        // Call MPI_alltoallv to send and receive data
        MPI_Alltoallv(sendbuf.data(), sendcounts, sdispls, MPI_INT,
                      recvbuf.data(), recvcounts, rdispls, MPI_INT,
                      MPI_COMM_WORLD);


        int total_edges_to_recv = 0;
        for(int i = 0;i<num_procs; i++) {
            total_edges_to_recv += recvbuf[3*i + 2];
        }
        vector<int> sendedgebuf;
        vector<int> recvedgebuf(3*total_edges_to_recv, -5);
        sedgedispls[0] = 0;
        redgedispls[0] = 0;
        bool flag=false;
        for(int i=0; i<num_procs; i++){
            if(i!=0)sedgedispls[i]=sedgedispls[i-1] + sendedgecounts[i-1]; 
            for( pair<pair<int, int>, int> e : sendingedges[i]){
                sendedgebuf.push_back(e.first.first);
                sendedgebuf.push_back(e.first.second);
                sendedgebuf.push_back(e.second);

            }
        }
        for(int i = 0; i<num_procs; i++) {
            
            int x1=recvbuf[3*i], y1=recvbuf[3*i+1];
            
            if(x1!=-1){
                if(G[x1].find(y1)!=G[x1].end()) {
                    G[x1].erase(y1);
                }
                if(G[y1].find(x1)!=G[y1].end()){
                    G[y1].erase(x1);
                }
            }
            else count++;
            
            recvedgecounts[i] = 3*recvbuf[3*i + 2];
            if(i > 0) {
                redgedispls[i] = redgedispls[i-1] + recvedgecounts[i-1];
            }
        }
    


        MPI_Alltoallv(sendedgebuf.data(), sendedgecounts, sedgedispls, MPI_INT,
                        recvedgebuf.data(), recvedgecounts, redgedispls, MPI_INT,
                        MPI_COMM_WORLD);

        set<pair<pair<int, int>, int>> handled;
            for(int it1 = 0; it1<recvedgebuf.size(); it1+=3) {
                pair<pair<int, int>, int> ed = { make_pair(recvedgebuf[it1], recvedgebuf[it1 + 1]), recvedgebuf[it1 + 2] };
                pair<int, int> ed1=ed.first;
                #ifdef gg
                if(ed1 == make_pair(3, 10)) cout<<my_rank<<": Yes"<<endl;
                #endif
                if(handled.find(ed)==handled.end()){
                    handled.insert(ed);
                    if(my_edges.find(ed1) != my_edges.end()) {
                        supp[ed1] --;
                        if(supp[ed1] < k-2 && my_deletable.find(ed1) == my_deletable.end()) {
                            my_deletable.insert(ed1);
                        }
                    }
                }
            }

        if(count == num_procs) {
            break;
        }

    }
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


    MPI_Init(&argc, &argv);


    //start
    int n,m;
    map<int,set<int> > adjList;
    readInputFromFile(inputpath,n,m,adjList);
    map<pair<int,int>,int> supp;
    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    ofstream outfile;
    if(myrank==0){
        outfile.open(outputpath, ios::out);
    }

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

        filterEdges(adjList,supp,k, deletable2);
        


        //output
        if(myrank == 0){
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
                        std::cout << "components size = " << components.size() << endl;
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
    }

    MPI_Finalize();
    return 0;
}