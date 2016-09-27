/**
@description
Cache Simulator for Computer Arch Homework II
@author Yukun Zeng
@uin 225009787
@institution Texas A&M University
**/

#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <cmath>

#define BIT 1
#define BYTE (8 * BIT) //this is also the length of a long long int/a memory address
#define KB (1024 * BYTE)
#define INFINITI 9223372036854775807

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
    long long int getCounter(){
      return counter;
    }

    long long int addr;
    bool validBit;
    long long int counter;
    //int data;
};

class Cache{
  public:
    //cache->cache set->cache block->cache piece
    vector<vector<vector<CachePiece> > > cachePieces;
    int setsInCache;
    int blocksInSet;
    int cachesInBlock;
    int setIndexLength;
    char repl;
    int offset;

    Cache(int nk, int assoc, int blocksize, char repl){
      //calculate some useful data for simulation
      setsInCache = (nk * KB) / (blocksize * BYTE * assoc);
      setIndexLength = log2(setsInCache);
      blocksInSet = assoc;
      cachesInBlock = (blocksize * BYTE) / (64 * BIT);
      offset = log2(blocksize);
      this->repl = repl;

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

    //if cache hit, return true, else false
    bool read(long long int addr){
      //remove the offset in the addr
      long long int blockAddr = addr / offset;
      long long int setIndex = blockAddr % setsInCache;
      long long int tag = blockAddr >> (setIndexLength);

      for(int i = 0; i < blocksInSet; i++){
        long long int tmpTag =
          (cachePieces[setIndex][i][0].addr/offset)>>(setIndexLength);
        //cout << i << " " << tmpTag << " " << tag << endl;
        if(tmpTag == tag){
          cachePieces[setIndex][i][0].counter ++;
          return true;
        }
      }
      //not hit
      //TODO replace or add block
      switch (repl){
        case 'l':
          replaceByLRU(addr, setIndex);
          break;
        case 'r':
          replaceByRandom(addr, setIndex);
          break;
      }
      return false;
    }

    void replaceByLRU(long long int addr, int setIndex){
      long long int minCounter = INFINITI;
      int minCounterIndex = -1;
      for(int i = 0; i < blocksInSet; i++){
        //first select those empty block, who's not valid now
        if(!(cachePieces[setIndex][i][0].validBit)){
          cachePieces[setIndex][i][0].addr = addr;
          cachePieces[setIndex][i][0].validBit = true;
          cachePieces[setIndex][i][0].counter = 1;
          return;
        }
        else{
          if(cachePieces[setIndex][i][0].counter < minCounter){
            minCounter = cachePieces[setIndex][i][0].counter;
            minCounterIndex = i;
          }
        }
      }
      cachePieces[setIndex][minCounterIndex][0].addr = addr;
      cachePieces[setIndex][minCounterIndex][0].counter = 1;
      return;
    }

    void replaceByRandom(long long int addr, int setIndex){
      int randIndex = rand() % blocksInSet;
      cachePieces[setIndex][randIndex][0].addr = addr;
      cachePieces[setIndex][randIndex][0].counter = 1;
      cachePieces[setIndex][randIndex][0].validBit = true;
    }
};

//block size in bytes
//nk in KB
string readTrace(const char* fileName){
  ifstream t(fileName);
  string str((istreambuf_iterator<char>(t)),
                       istreambuf_iterator<char>());
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
  char* fileName = argv[5];
  //cout << nk << "\n";
  //cout << blocksize << "\n";
  //cout << (nk / blocksize) << "\n";
  Cache *cache = new Cache(nk, assoc, blocksize, repl);
  string trace = readTrace(fileName);
  vector<string> traceVector;
  split(trace, '\n', traceVector);
  //test if get file input as vector string
  //cout << traceVector[0] << endl;
  int total = 0, hit = 0, miss = 0;
  int read = 0, write = 0;
  int readHit = 0, readMiss = 0, writeHit = 0, writeMiss = 0;
  //traceVector.size()
  for(int i = 0; i < traceVector.size(); i++){
    vector<string> traceLineVector;
    split(traceVector[i], ' ', traceLineVector);
    //cout << "This is the " << i << "th operation:" << endl;
    //cout << traceLineVector[0] << " " << traceLineVector[1] << endl;
    char operation = traceLineVector[0].at(0);
    long long int addr = stoll(traceLineVector[1], NULL, 16);
    //cout << "1st read: " << cache->read(addr) << endl;
    //cout << "2nd read: " << cache->read(addr) << endl;
    if(cache->read(addr)){
      hit++;
      if(operation == 'r'){
        readHit ++;
        read ++;
      }
      else{
        writeHit ++;
        write ++;
      }
    }
    else{
      miss ++;
      if(operation == 'r'){
        readMiss ++;
        read ++;
      }
      else{
        writeMiss ++;
        write ++;
      }
    }
    total++;
  }
  double totalR = ((double)miss)/total * 100;
  double readR = ((double)readMiss)/read * 100;
  double writeR = ((double)writeMiss/write) * 100;
  cout << miss << " " << totalR << "% " << readMiss << " " << readR << "% " <<
    writeMiss << " " << writeR << "% " << endl;
}
