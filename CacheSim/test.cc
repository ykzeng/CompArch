#include <vector>
#include <iostream>
#include <string>

using namespace std;

class test{
  public:
    int test1;
    test(){
      test1 = 0;
      cout << "constructing!\n";
      cout << "current target is " << this->test1 << endl;
    }
};

int main(void){
  vector<string> lines;
  string tmp;
  for(int i = 0; getline(cin, tmp); i++){
    lines.resize(i+1);
    lines[i] = tmp;
  }
  cout << lines[0] << endl;
  //long long int i = stoll("0x1234AFE", NULL, 16);
  //cout << "The number is " << i;
  //vector<vector<vector<test > > > array;
  //array.resize(2);
  //for (int i = 0; i < 2; i++){
  //  array[i].resize(2);
  //  for (int j = 0; j < 2; j++){
  //    array[i][j].resize(2);
  //    //for (int k = 0; k < 10; k++){
  //    //  array[i][j][k] = new test();
  //    //}
  //  }
  //}
  //cout << array[9][9][9].test1;
}
