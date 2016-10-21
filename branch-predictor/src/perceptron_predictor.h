// path_perceptron_predictor.h
// This file contains a sample path_perceptron_predictor class.
// It is a simple 32,768-entry gshare with a history length of 15.
// Note that this predictor doesn't use the whole 8 kilobytes available
// for the CBP-2 contest; it is just an example.
#define H_LENGTH  37
#define N 2048
#define TABLE_BITS	15
#define THETA (2.14 * (H_LENGTH + 1) + 20.58)
#define TAKEN true
#define NOT_TAKEN false

class path_perceptron_update : public branch_update {
  //super:
  //prediction results including: direction prediction, target prediction
  //y result to compare with theta
  int y_out;
  //current address modulo n
  int i;
  //integers representing the addr of the last h branches predictred modulo n
  int* v;
  //speculative global history
  bool* h;
  public:
    path_perceptron_update(){
      init(H_LENGTH);
    }
    path_perceptron_update(int h_length){
      init(h_length);
    }

    void init(int h_length){
      int tmp_v[h_length];
      bool tmp_h[h_length];
      memset(tmp_v, 0, sizeof(int) * h_length);
      memset(tmp_h, true, sizeof(bool) * h_length);
      v = tmp_v;
      h = tmp_h;
    }
};

class path_perceptron_predictor : public branch_predictor {

  public:
    //TODO
    //number of weight vectors, history length, bits per weight
    //training threshold, max weight value, min weight value
    int n, h, b, max_weight, min_weight;
    //weight matrix
    int w[n][h + 1];
    //shift speculative global history register
    //shift global history register
    bool sg[h + 1], g[h + 1];
    //speculative partial sum from last prediction
    //non-speculative partial sum from last predicion
    int sr[h + 1], r[h + 1];
    //addresses of the last h branches predicted modulo n
    //speculative v
    int v[h + 1], sv[h + 1];
    //maximum weight & minimum weight in our weight matrix
    int max_weight;
    int min_weight;

    path_perceptron_update u;
    branch_info bi;




    path_perceptron_predictor (void) {
      int length = H_LENGTH + 1;
      //init all arrays
      memset(g, true, (sizeof(g)));
      memset(sg, true, (sizeof(sg)));
      memset(w, 0, (sizeof(w)));
      memset(sr, 0, (sizeof(sr)))
      memset(r, 0, (sizeof(r)));
      memset(v, 0, (sizeof(v)));
      memset(sv, 0, (sizeof(sv)));


    }

    path_perceptron_predictor(){

    }

    branch_update *predict (branch_info & b) {
      path_perceptron_update u;
      //i: addr modulo n, j: loop counter
      //k: shift vector index, y y_out
      int i, j, k, y;
      //prediction result
      bool prediction;
      bi = b;
      //change_possible
      i = bi.address % n;
      //shift new addr into history addr array
      shift_int(s_v, h, i);
      //int y = sr[HISTORY_LENGTH] + w[i][0];
      //bool prediction = TAKEN;
      //int k_j = 0;
      //int tmp_sr[HISTORY_LENGTH];
      //memset(tmp_sr, 0, sizeof(tmp_sr));


      //if (y >= 0)
      //  prediction = TAKEN;
      //else
      //  prediction = NOT_TAKEN;
      //for (j = 1; j < HISTORY_LENGTH; j++){
      //  k_j = HISTORY_LENGTH - j;
      //  if (prediction == TAKEN)
      //    tmp_sr[k_j + 1] = sr[k_j] + w[i][j];
      //  else
      //    tmp_sr[k_j + 1] = sr[k_j] - w[i][j];
      //}
      //memcpy(sr, tmp_sr, sizeof(tmp_sr));
      ////sr[0] = 0;
      //sg = ((sg << 1) | prediction);
    }

    //@branch_update: the information needed to do branch update
    //@taken: the outcome of real execution
    //@target: ommitted, we are not going to do branch target prediciton
    void update (branch_update *u, bool taken, unsigned int target) {
      path_perceptron_update* u = (path_perceptron_update*)branch_update;
      bool outcome = taken;
      bool prediction = u->direction_prediction();

      if ((prediction != outcome) || (this->abs(y_out) <= THETA)){
        w[i][0] = w[i][0] + (outcome == taken? 1 : -1);
        int j = 1;
        for (j = 1; j < HISTORY_LENGTH; j++){
          int k = v[j];
          w[k][j] = w[k][j] + (outcome == )
        }
      }
    }

    unsigned int abs(int n){
      int ret[] = {n, -n};
      return ret[n < 0];
    }

    int[] shift_int(int[] array, int length, int number){
      int j = 0;
      for (j = length - 1; j > 0; j++){
        array[j] = array[j - 1];
      }
      j[0] = number;
    }
};
