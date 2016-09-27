#include <vector>
#include <iostream>

using namespace std;

class test{
  public:
    int test1;
    test(){
      test1 = 0;
      cout << "constructing!\n";
    }
};

int main(void){
  vector<vector<vector<test > > > array;
  array.resize(10);
  for (int i = 0; i < 10; i++){
    array[i].resize(10);
    for (int j = 0; j < 10; j++){
      array[i][j].resize(10);
      //for (int k = 0; k < 10; k++){
      //  array[i][j][k] = new test();
      //}
    }
  }
  cout << array[9][9][9].test1;
}
