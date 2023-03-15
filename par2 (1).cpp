#include <bits/stdc++.h>
#include <fstream>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <mpi.h>
// #include <omp.h>

// #define INPUT_DEBUG 1
// #define DEBUG_COMP 1
// #define RECV_DAT 1
// #define SEND_DAT 1
//#define MY_DEL 1
//#define gg 1



using namespace std;

struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator () (std::pair<T1, T2> const &pair) const
    {
        std::size_t h1 = std::hash<T1>()(pair.first);
        std::size_t h2 = std::hash<T2>()(pair.second);
 
        return h1 ^ h2;
    }
};


float comp_time = 0.0;
float total_time = 0.0;

string get_arg(char* str) {
    string a = str;
    int s = a.find("=");
    return a.substr(s+1, a.length() - s - 1);
}

void print_graph(vector<vector<int>> &neighbours) {
    for(int i = 0; i<neighbours.size(); i++) {
        cout<<i<<" -> ";
        for(int e: neighbours[i]) {
            cout<<e<<", ";
        }
        cout<<endl;
    }
    return;
}

void input_from_file(char* inputPath, vector<vector<int>> &neighbours) {
    // cout<<"Hello";
    FILE *fp=fopen(inputPath,"rb");
    if (!fp) {
        cout<<"Wrong input path\n";
        return;
	}

    unsigned char bytes[4];

    // n = number of vertices
    fread(bytes, 4, 1,fp);
    int n = bytes[0] | (bytes[1]<<8) | (bytes[2]<<16) | (bytes[3]<<24);

    #ifdef INPUT_DEBUG
    cout<<n<<" ";
    #endif

    // m = number of edges
    fread(bytes, 4, 1,fp);
    int m = bytes[0] | (bytes[1]<<8) | (bytes[2]<<16) | (bytes[3]<<24);
    
    #ifdef INPUT_DEBUG
    cout<<m<<endl;
    #endif

    // vector<vector<int>> neighbours;
    // vector<vector<int>> neighbours(n);
    neighbours.resize(n);
    for(int i = 0; i<n; i++) {
        fread(bytes, 4, 1,fp);
        int ver1 = bytes[0] | (bytes[1]<<8) | (bytes[2]<<16) | (bytes[3]<<24);
        #ifdef INPUT_DEBUG
        cout<<ver1<<" ";
        #endif
        fread(bytes, 4, 1,fp);
        int deg1 = bytes[0] | (bytes[1]<<8) | (bytes[2]<<16) | (bytes[3]<<24);
        #ifdef INPUT_DEBUG
        cout<<deg1<<" ";
        #endif

        for(int j = 0; j<deg1; j++) {
            fread(bytes, 4, 1,fp);
            int ver2 = bytes[0] | (bytes[1]<<8) | (bytes[2]<<16) | (bytes[3]<<24);
            neighbours[ver1].push_back(ver2);
            #ifdef INPUT_DEBUG
            cout<<ver2<<" ";
            #endif
        }
        #ifdef INPUT_DEBUG
        cout<<endl;
        #endif
    }
    fclose(fp);
    return;
}

void prefilter(vector<vector<int>>& G, int k) {
    vector<int> deletable;

    #ifdef DEBUG_COMP
    cout<<"Prefilter verices: ";
    #endif

    // Find all vertices with degree less than k-1
    for (int v = 0; v < G.size(); v++) {
        if (G[v].size() < k-1) {
            deletable.push_back(v);
            #ifdef DEBUG_COMP
            cout<<v<<", ";
            #endif
        }
    }

    while (!deletable.empty()) {
        int v = deletable.back();
        deletable.pop_back();

        // Delete vertex v from graph
        for (int u : G[v]) {
            // Remove v from the list of neighbours of u
            auto it = find(G[u].begin(), G[u].end(), v);
            if(it!=G[u].end()) G[u].erase(it);

            // Queue neighbors for elimination if their degree dips below k
            if (G[u].size() < k-1) {
                deletable.push_back(u);
                #ifdef DEBUG_COMP
                cout<<u<<", ";
                #endif
            }
        }
        G[v].clear();
    }
    #ifdef DEBUG_COMP
    cout<<endl;
    #endif
    return;
}



void filterEdges(vector<vector<int>>& G, int k) {
    vector<pair<int, int>> deletable;
    map<pair<int, int>, int> support;
    map<pair<int, int>, bool> my_edge_map;

    int my_rank, num_procs;
    MPI_Comm_rank( MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size (MPI_COMM_WORLD, &num_procs);

    #ifdef DEBUG_COMP
    cout<<my_rank<<": ### k="<<k<<", Starting filterEdges graph ###\n";
    print_graph(G);
    #endif

    // Find all edges with support less than k-2
    // #ifdef MY_DEL
    // if(my_rank==0)cout<<my_rank<<": Filter edges("<<k-2<<"): "<<endl;
    // #endif

    for (int a = 0; a < G.size(); a++) {
        for (int b : G[a]) {
            if (a < b) {
                int sup = 0;
                for (int c : G[a]) {
                    if (c != b && find(G[b].begin(), G[b].end(), c) != G[b].end()) {
                        sup++;
                    }
                }
                support[make_pair(a, b)] = sup;
                if (sup < k-2) {
                    deletable.emplace_back(a, b);
                    #ifdef DEBUG_COMP
                    // cout<<"("<<a<<", "<<b<<"), ";
                    #endif
                }
            }
        }
    }

    // #ifdef DEBUG_COMP
    // cout<<"$ ";
    // cout<<"support[make_pair(13, 14)] = "<<support[make_pair(13, 14)]<<endl;
    // cout<<"support[make_pair(5, 13)] = "<<support[make_pair(5, 13)]<<endl;
    // cout<<"support[make_pair(3, 10)] = "<<support[make_pair(3, 10)]<<endl;
    // #endif


    // FilterEdges(G, k): (Eliminate edges with low support)

    // Remove edges with low support and update support counts of other edges

    // Create an edge set
    vector<pair<int, int>> edges;
    for(int u = 0; u<G.size(); u++){
        for(int v_idx = 0; v_idx<G[u].size(); v_idx++) {
            if(u<G[u][v_idx]) {
                edges.push_back(make_pair(u, G[u][v_idx]));
            }
        }
    }
    // Partition the edges based on number of processes
    
    // int my_chunk_size = edges.size()/num_procs + (((edges.size()%num_procs) >= (my_rank + 1))? 1:0);
    unordered_set<pair<int, int>, pair_hash> my_edges;
    // cout<<"Rank "<<my_rank<<": "<< ", Total edges ="<<edges.size()<<endl;
    
    map<pair<int, int>, int> ownerp;

    for(int it1 = my_rank; it1<edges.size(); it1+=num_procs) {
        my_edges.insert(edges[it1]);
        my_edge_map[edges[it1]] = true;
        // cout<<"edge "<<it1<<endl;
    }

    for(int i=0; i<edges.size(); i++){
        ownerp[edges[i]]=i%num_procs;
    }
    // if(my_rank == 1) {
    // cout<<"Rank "<<my_rank<<": my_edges = {";
    // for(auto e: my_edges) {
    //     cout<<"("<<e.first<<", "<<e.second<<"), ";
    // }
    // cout<<"}"<<endl;
    // }

    unordered_set<pair<int, int>, pair_hash> my_deletable;

    for(int i = 0; i<deletable.size(); i++) {
        if(my_edges.find(deletable[i])!=my_edges.end()) {
            my_deletable.insert(deletable[i]);
            // #ifdef MY_DEL
            // cout<<"("<<deletable[i].first<<", "<<deletable[i].second<<"), ";
            // #endif
        }
    }

    // Parallel code
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

    while(!all_fin) {
        vector<int> recvbuf(3*num_procs);
        vector<int> sendbuf(3*num_procs);
        set<pair<pair<int, int>, int>> sendingedges[num_procs];
        
        for(int i=0; i<num_procs; i++)sendedgecounts[i]=0; //reset edge count to be sent
        
        #ifdef DEBUG_COMP
        // cout<<"Hi"<<endl;
        #endif

        int count = 0;
        if(my_deletable.empty() == false) {
           
            auto e = *my_deletable.begin();

            #ifdef gg
            if(e==make_pair(3, 11))cout<<"deleting {3, 11}"<<endl;
            #endif
            my_deletable.erase(e);
            my_edges.erase(e);
            my_edge_map[e] = false;
            #ifdef MY_DEL
            cout<<k-2<<"-> Rank "<<my_rank<<" deletes : "<<"("<<e.first<<", "<<e.second<<"), support[make_pair(3, 10)] ="<<support[make_pair(3, 10)]<<endl;
            #endif
            
            int x1 = e.first, y1 = e.second;
            for(int a: G[x1]) {
                if(find(G[y1].begin(), G[y1].end(), a) != G[y1].end()) {
                    
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


        #ifdef SEND_DAT
        cout<<"Rank "<<my_rank<<" send_count: ";
        for(int i = 0; i<num_procs; i++) {
            cout<<"("<<sendbuf[3*i]<<", "<<sendbuf[3*i + 1]<<")["<<sendbuf[3*i+2]<<"], ";
        }
        cout<<"$"<<endl;

        // cout<<my_rank<<": Before A2Av:"<<my_rank<<endl;
        #endif


        // Call MPI_alltoallv to send and receive data
        MPI_Alltoallv(sendbuf.data(), sendcounts, sdispls, MPI_INT,
                      recvbuf.data(), recvcounts, rdispls, MPI_INT,
                      MPI_COMM_WORLD);
        
        // #ifdef RECV_DAT
        // if(my_rank == 1) {
        // cout<<"Rank "<<my_rank<<" recv_count: ";
        // for(int i = 0; i<num_procs; i++) {
        //     cout<<"("<<recvbuf[3*i]<<", "<<recvbuf[3*i + 1]<<")["<<recvbuf[3*i+2]<<"], ";
        // }
        // cout<<"$"<<endl;
        // }

        // cout<<my_rank<<": Before A2Av:"<<my_rank<<endl;
        // #endif

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
                #ifdef gg
                if(e==make_pair(3,10))flag=true;
                #endif
                sendedgebuf.push_back(e.first.first);
                sendedgebuf.push_back(e.first.second);
                sendedgebuf.push_back(e.second);

            }
        }
        for(int i = 0; i<num_procs; i++) {
            
            int x1=recvbuf[3*i], y1=recvbuf[3*i+1];
            
            if(x1!=-1){
                if(find(G[x1].begin(), G[x1].end(), y1)!=G[x1].end())
                    G[x1].erase(find(G[x1].begin(), G[x1].end(), y1));
                if(find(G[y1].begin(), G[y1].end(), x1)!=G[y1].end())
                    G[y1].erase(find(G[y1].begin(), G[y1].end(), x1));
            }
            else count++;
            
            recvedgecounts[i] = 3*recvbuf[3*i + 2];
            if(i > 0) {
                redgedispls[i] = redgedispls[i-1] + recvedgecounts[i-1];
            }
        }

        #ifdef SEND_DAT
        cout<<"Rank "<<my_rank<<" send_data: ";
        for(int i = 0; i<sendedgebuf.size(); i+=3) {
            cout<<"("<<sendedgebuf[i]<<", "<<sendedgebuf[i + 1]<<"), ";
        }
        cout<<"$"<<endl;

        // cout<<my_rank<<": Before A2Av:"<<my_rank<<endl;
        #endif
        
        #ifdef gg
        if(flag){
            cout<<my_rank<<": contents of sendedgebuf:"<<endl;
            for(int i=0; i<sendedgebuf.size(); i+=3){
                cout<<sendedgebuf[i]<<", "<<sendedgebuf[i+1]<<endl;
            }
            cout<<my_rank<<": contents of sendedgecounts:"<<endl;
            for(int i=0; i<num_procs; i++)cout<<sendedgecounts[i]<<", ";
            cout<<endl;
            cout<<my_rank<<": contents of disp: "<<endl;
            for(int i=0; i<num_procs; i++)cout<<sedgedispls[i]<<", ";
            cout<<endl;
        }
        #endif
        MPI_Alltoallv(sendedgebuf.data(), sendedgecounts, sedgedispls, MPI_INT,
                      recvedgebuf.data(), recvedgecounts, redgedispls, MPI_INT,
                      MPI_COMM_WORLD);
        
        #ifdef RECV_DAT
        
        cout<<"Rank "<<my_rank<<" recv_data: ";
        for(int i = 0; i<recvedgebuf.size(); i+=2) {
            cout<<"("<<recvedgebuf[i]<<", "<<recvedgebuf[i + 1]<<"), ";
        }
        cout<<"$"<<endl;
        

        // cout<<my_rank<<": Before A2Av:"<<my_rank<<endl;
        #endif
        // if(my_rank==1){
        //     cout<<k-2<<"-> ";
        //     for(int i=0; i<recvedgebuf.size(); i++)cout<<recvedgebuf[i]<<", ";
        //     cout<<endl;
        // }
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
                    support[ed1] --;
                    if(support[ed1] < k-2 && my_deletable.find(ed1) == my_deletable.end()) {
                        my_deletable.insert(ed1);
                    }
                }
            }
        }

        if(count == num_procs) {
            break;
        }
    }

    

    #ifdef DEBUG_COMP
    cout<<endl;
    #endif
    // cout<<my_rank<<": hogya bhai"<<endl;
    return;
}

void dfs(vector<vector<int>>& G, vector<bool> &visited, int v, vector<int> &verts) {
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


void output_to_file(vector<vector<int>> &neighbours, char* outputPath, int verbose, int startk, int endk) {
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    ofstream outHandle;
    if(my_rank == 0) {
        outHandle.open(outputPath, std::ofstream::out);
        if (!outHandle.is_open()) {
            cout << "Problem with opening file";
            return;
        }
    }
    // auto end = std::chrono::high_resolution_clock::now();
    // float ms = (1e-6 * (std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)).count());
    // cout <<"Time for computation: "<< ms << " ms\n";

    for(int i = startk; i<=endk; i++) {
        std::chrono::_V2::system_clock::time_point begin;

        if(my_rank == 0) {
            #ifdef DEBUG_COMP
            cout<<"### k="<<i<<" initial graph ###\n";
            print_graph(neighbours);
            #endif

            begin = std::chrono::high_resolution_clock::now();
        }

        prefilter(neighbours, i);
        filterEdges(neighbours, i);

        if(my_rank == 0) {
            auto end = std::chrono::high_resolution_clock::now();
            float ms = (1e-6 * (std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)).count());
            comp_time+=ms;

            vector<bool> visited(neighbours.size(), false);
            vector<vector<int>> vs;

            for(int j = 0; j<neighbours.size(); j++) {
                vector<int> verts;
                dfs(neighbours, visited, j, verts);
                sort(verts.begin(), verts.end());
                if(verts.size() >= 2) vs.push_back(verts);
            }
            outHandle<<((vs.size()>0)?1:0)<<endl;
            outHandle<<vs.size()<<endl;
            if(verbose == 1) {
                for(auto e1: vs) {
                    // cout<<"e1.size() = "<<e1.size()<<endl;
                    for(auto e2: e1) {
                        outHandle<<e2<<" ";
                    }
                    outHandle<<endl;
                }
            }
        }
    }
    if(my_rank == 0) {
        outHandle.close();
    }

    //Printing computation status
    #ifdef DEBUG_COMP
    {
        int my_rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

        if(my_rank == 0)
            cout<<"startk = "<< startk<<", endk = "<<endk<<endl;

        for(int i = startk; i<=endk; i++) {
            // cout<<"startk = "<< startk<<", endk = "<<endk<<endl;


            #ifdef DEBUG_COMP
            cout<<"### k="<<i<<" initial graph ###\n";
            print_graph(neighbours);
            #endif

            prefilter(neighbours, i);
            filterEdges(neighbours, i);
            cout<<my_rank<<": out of filterEdges"<<endl;


            // cout<<"neighbours[0].size() = " << neighbours[0].size()<<endl;

            if(my_rank == 0) {
                vector<bool> visited(neighbours.size(), false);

                vector<vector<int>> vs;

                for(int j = 0; j<neighbours.size(); j++) {
                    vector<int> verts;
                    dfs(neighbours, visited, j, verts);
                    sort(verts.begin(), verts.end());
                    if(verts.size() >= 2) vs.push_back(verts);
                    // cout<<"verts.size() = "<<verts.size()<<endl;
                    // if(verts.size() == i) {
                    //     vs.emplace_back(verts);
                    // }
                }
                cout<<vs.size()<<endl;
                if(verbose == 1) {
                    for(auto e1: vs) {
                        // cout<<"e1.size() = "<<e1.size()<<endl;
                        for(auto e2: e1) {
                            cout<<e2<<" ";
                        }
                        cout<<endl;
                    }
                }
            }
        }
    }
    #endif

    #ifdef DEBUG_COMP
    if(my_rank == 0)  {
        cout<<"### Final graph ###\n";
        print_graph(neighbours);
    }
    #endif

    return;
}

int main(int argc, char* argv[]) {
    string argvals[argc];

    #ifdef INPUT_DEBUG
    printf("Number of arguments = %d\n", argc);
    #endif


    MPI_Init(&argc, &argv);

    for(int i = 0; i<argc; i++) {
        argvals[i] = get_arg(argv[i]);
        #ifdef INPUT_DEBUG
        printf("argv[%d] = %s, len = %ld, argval = ", i, argv[i], strlen(argv[i]));
        cout<<argvals[i]<<endl;
        #endif
    }

    // Number of arguments = 9
    // arg[0] = ./a2, len = 4, argval = ./a2
    // arg[1] = --taskid=1, len = 10, argval = 1
    // arg[2] = --inputpath=abc, len = 15, argval = abc
    // arg[3] = --headerpath=bcd, len = 16, argval = bcd
    // arg[4] = --outputpath=out, len = 16, argval = out
    // arg[5] = --verbose=0, len = 11, argval = 0
    // arg[6] = --startk=0, len = 10, argval = 0
    // arg[7] = --endk=1, len = 8, argval = 1
    // arg[8] = --p=3, len = 5, argval = 3

    // ./a2 --taskid=1 --inputpath=/home/roshan/Desktop/Sem6/COL380/COL380_A2/testcases/new/test0/test-input-0.gra --headerpath=/home/roshan/Desktop/Sem6/COL380/COL380_A2/testcases/new/test0/test-header-0.dat --outputpath=out --verbose=1 --startk=1 --endk=10 --p=3

    // INPUT PATH Done
    char *inputPath = new char[argvals[2].length() + 1];
    inputPath[argvals[2].length()] = '\0';
    for (int i = 0; i < argvals[2].length(); i++) inputPath[i] = argvals[2][i];

    // Output Path Done
    #ifdef INPUT_DEBUG
    cout<<"verbose = "<<argv[5]<<atoi(get_arg(argv[5]).c_str())<<endl;
    cout<<"startk = "<<argv[6]<<atoi(get_arg(argv[6]).c_str())<<endl;
    cout<<"endk = "<<argv[7]<<atoi(get_arg(argv[7]).c_str())<<endl;
    #endif

    char *outputPath = new char[argvals[4].length() + 1];
    outputPath[argvals[4].length()] = '\0';
    for (int i = 0; i < argvals[4].length(); i++) outputPath[i] = argvals[4][i];


    auto begin = std::chrono::high_resolution_clock::now();
    vector<vector<int>> neighbours;
    input_from_file(inputPath, neighbours);
    if(neighbours.size() == 0) {
        // No input received
        return 0;
    }
    // vector<vector<int>> neighbours = {{1, 10}, {0, 2, 10, 12}, {1, 3, 11}, \
    //                             {2, 4, 10, 11}, {3, 6, 10, 12, 13}, {12, 13, 14}, \
    //                             {4, 7, 12, 14}, {6, 8, 10}, {7, 9, 10, 13, 14}, {8, 10, 14}, \
    //                             {0, 1, 3, 4, 7, 8, 9, 11, 12, 13, 14}, {2, 3, 10, 12, 13}, \
    //                             {1, 4, 5, 6, 10, 11, 13}, {4, 5, 8, 10, 11, 12, 14}, \
    //                             {5, 6, 8, 9, 10, 13}};
    // cout<<"Input Done"<<endl;
    for(int i = 0; i<neighbours.size(); i++) {
        sort(neighbours[i].begin(), neighbours[i].end());
    }

    #ifdef DEBUG_COMP
    cout<<"### Initial graph ###\n";
    print_graph(neighbours);
    cout<<"Going to Computation"<<endl;
    #endif
    
    output_to_file(neighbours, outputPath, atoi(get_arg(argv[5]).c_str()), atoi(get_arg(argv[6]).c_str())+2, atoi(get_arg(argv[7]).c_str())+2);
    auto end = std::chrono::high_resolution_clock::now();
    float total_time = (1e-6 * (std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)).count());
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Finalize();
    if(my_rank == 0) {
        cout <<"Time for computation: "<< comp_time << " ms\n";
        cout <<"Total Execution Time: "<< total_time << " ms\n";
    }

    return 0;
}