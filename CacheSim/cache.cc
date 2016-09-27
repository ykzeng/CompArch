#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <fstream>
#include <streambuf>
#include <sstream>

#define BIT 1
#define BYTE (8 * BIT) //this is also the length of a long long int/a memory address
#define KB (1024 * BYTE)

using namespace std;

//the class that denotes a piece of cache
class CachePiece{
  public:
    //constructor, init all data in a piece of cache to be meaningless
    CachePiece(){
      addr = 0x0;
      validBit = false;
      counter = 0;
    }
    int getCounter(){
      return counter;
    }
  private:
    long long int addr;
    bool validBit;
    int counter;
    //int data;
};

class Cache{
  private:
    //cache->cache set->cache block->cache piece
    vector<vector<vector<CachePiece> > > cachePieces;
  public:
    Cache(int nk, int assoc, int blocksize){
      //calculate some useful data for simulation
      int setsInCache = (nk * KB) / (blocksize * BYTE * assoc);
      int blocksInSet = assoc;
      int cachesInBlock = (blocksize * BYTE) / (64 * BIT);

      cachePieces.resize(setsInCache);
      for(int i = 0; i < setsInCache; i++){
        cachePieces[i].resize(blocksInSet);
        for(int j = 0; j < blocksInSet; j++){
          //through this, the vector call default constructor in class
          cachePieces[i][j].resize(cachesInBlock);
        }
      }
      //test if the cachePieces array is initialized
      //cout << cachePieces[0][0][0].getCounter() << "\n";
    }
};

//class Cache{
//  private:
//    CachePiece ***cachePieces;
//
//  public:
//    Cache(int nk, int assoc, int blocksize){
//      int setsInCache = (nk * KB) / (blocksize * BYTE * assoc);
//      int blocksInSet = assoc;
//      int cachesInBlock = (blocksize * BYTE) / (64 * BIT);
//
//      cachePieces = new CachePieces **[setsInCache];
//      for(int i = 0; i < setsInCache; i++){
//        cachePieces[i] = new CachePieces *[blockInSet];
//        for(int j = 0; j < blocksInSet; j++){
//          cachePieces[i][j] = new CachePieces[cachesInBlock];
//          for(int k = 0; k < cachesInBlock; k++){
//            cachePieces[i][j][k] = new CachePieces();
//          }
//        }
//      }
//
//      cout << cachePieces[0][0][0].getCounter() << "\n";
//    }
//};

//class Cache{
//public:
//  long long int **cache;
//
//  Cache(int nk, int blocksize){
//    int blockCount = ((nk * KB) / (blocksize * BYTE));
//    int addrCount = (blocksize * BYTE) / (64 * BIT);
//    cout << "Hello, world!\n";
//    cout << blockCount << " " << addrCount;
//    cache = new long long int*[blockCount];
//    for(int i = 0; i < blockCount; i++)
//      cache[i] = new long long int[addrCount];
//    cache[0][0] = 0x56ecd8;
//    cout << "The 0th addr in block 0 is: " << cache[0][0] << "\n";
//    cache[blockCount-1][addrCount-1]=0x56ecd9;
//    cout << "The" << blockCount-1 << "th addr in block"
//      << addrCount - 1 << "is: " << cache[blockCount-1][addrCount-1] << "\n";
//  }
//};

//block size in bytes
//nk in KB
string readTrace(const char* fileName){
  std::ifstream t(fileName);
  std::string str((std::istreambuf_iterator<char>(t)),
                       std::istreambuf_iterator<char>());
  return str;
}

void split(const string &s, char delim, vector<string> &elems) {
  stringstream ss;
  ss.str(s);
  string item;
  while (getline(ss, item, delim)) {
    elems.push_back(item);
  }
}

int main(int argc, char **argv){
  //long long int **a = new long long int*[5];
  //for(int i = 0;i<5;i++)
  //      a[i] = new long long int[5];
  //a[0][0] = 1;
  //a[4][4] = 1;
  //cout << a[0][0] << " " << a[4][4];
  //param processing
  int nk = atoi(argv[1]);
  int assoc = atoi(argv[2]);
  int blocksize = atoi(argv[3]);
  char repl = argv[4][0];
  //cout << nk << "\n";
  //cout << blocksize << "\n";
  //cout << (nk / blocksize) << "\n";
  Cache *cache = new Cache(nk, assoc, blocksize);
  string trace = readTrace("trace.txt");
  vector<string> traceVector;
  split(trace, '\n', traceVector);
  //test if get file input as vector string
  //cout << traceVector[0] << endl;
  for(int i = 0; i < traceVector.size(); i++){

  }
}
