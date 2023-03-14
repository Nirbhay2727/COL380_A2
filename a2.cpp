#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

void readInputFromFile(const string& filename, int& n, int& m, vector<vector<int>>& adjList,vector<pair<int, int>>& edges) {
    ifstream infile(filename, ios::binary); // open the input file in binary mode
    if (!infile.is_open()) { // check if the file is successfully opened
        cerr << "Error: Failed to open file \"" << filename << "\"" << endl;
        exit(1); // exit the program with an error code
    }

    infile.read(reinterpret_cast<char*>(&n), sizeof(n)); // read the number of nodes in little endian format
    n = __builtin_bswap32(n); // convert the integer from little endian to host byte order
    adjList.resize(n); // resize the adjacency list vector to store n nodes

    infile.read(reinterpret_cast<char*>(&m), sizeof(m)); // read the number of nodes in little endian format
    m = __builtin_bswap32(m); // convert the integer from little endian to host byte order


    for (int i = 0; i < n; i++) {
        int node,k;
        infile.read(reinterpret_cast<char*>(&node), sizeof(node)); // read the number of nodes in little endian format
        node = __builtin_bswap32(node); // convert the integer from little endian to host byte order
        infile.read(reinterpret_cast<char*>(&k), sizeof(k)); // read the number of nodes in little endian format
        k = __builtin_bswap32(k); // convert the integer from little endian to host byte order
        adjList[node].resize(n); // resize the vector for node i to store m neighbors
        for (int j = 0; j < k; j++) {
            int temp;
            infile.read(reinterpret_cast<char*>(&temp), sizeof(temp)); // read the number of nodes in little endian format
            temp = __builtin_bswap32(temp); // convert the integer from little endian to host byte order
            adjList[node][temp]=1; // read the j-th neighbor of node i
            edges.push_back(make_pair(temp,i));
        }
    }
    infile.close(); // close the input file
}


void calculateSupp(){

}
int main(int argc, char* argv[]) {
    vector<pair<int, int>> edges;
    int n,m;
    string inputpath="";
    vector<vector<int>> adjList;
    readInputFromFile(inputpath,n,m,adjList,edges);
    







    return 0;
}