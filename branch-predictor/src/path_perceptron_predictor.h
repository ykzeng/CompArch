// path_perceptron_predictor.h
// This file contains a sample path_perceptron_predictor class.
// It is a simple 32,768-entry gshare with a history length of 15.
// Note that this predictor doesn't use the whole 8 kilobytes available
// for the CBP-2 contest; it is just an example.
#define H_LEN  37
#define N 2048
#define THETA (int)(2.14 * (H_LEN + 1) + 20.58)
#define TAKEN true
#define NOT_TAKEN false

class path_perceptron_update : public branch_update {
  //super:
  //prediction results including: direction prediction, target prediction
  public:
    //y result to compare with theta
    int y;
    //current address modulo n
    int i;
    //integers representing the addr of the last h branches predictred modulo n
    int v[H_LEN + 1];
    //speculative global history
    bool h[H_LEN + 1];
    path_perceptron_update(){
      //init(H_LEN);
      memset(v, 0, sizeof(v));
      memset(h, NOT_TAKEN, sizeof(h));
    }
    //path_perceptron_update(int h_length){
    //  init(h_length);
    //}

    //void init(int h_length){
    //  int tmp_v[h_length];
    //  bool tmp_h[h_length];
    //  memset(tmp_v, 0, sizeof(int) * h_length);
    //  memset(tmp_h, true, sizeof(bool) * h_length);
    //  v = tmp_v;
    //  h = tmp_h;
    //}
};

class path_perceptron_predictor : public branch_predictor {

  public:
    //TODO
    //number of weight vectors, history length, bits per weight
    //training threshold, max weight value, min weight value
    int n, h, b, max_weight, min_weight;
    //weight matrix
    int w[N][H_LEN + 1];
    //shift speculative global history register
    //shift global history register
    bool sg[H_LEN + 1], g[H_LEN + 1];
    //speculative partial sum from last prediction
    //non-speculative partial sum from last predicion
    int sr[H_LEN + 1], r[H_LEN + 1];
    //addresses of the last h branches predicted modulo n
    //speculative v
    int v[H_LEN + 1], sv[H_LEN + 1];

    path_perceptron_update u;
    branch_info bi;




    path_perceptron_predictor (void) {
      //init all arrays
      memset(g, NOT_TAKEN, (sizeof(g)));
      memset(sg, NOT_TAKEN, (sizeof(sg)));
      memset(w, 0, (sizeof(w)));
      memset(sr, 0, (sizeof(sr)));
      memset(r, 0, (sizeof(r)));
      memset(v, 0, (sizeof(v)));
      memset(sv, 0, (sizeof(sv)));

      this->max_weight = 127;
      this->min_weight = -128;
    }

    branch_update *predict (branch_info & b) {
      //i: addr modulo n, j: loop counter
      //k: shift vector index, y y_out
      int i, j, k, y;
      //prediction result
      bool prediction;
      bi = b;
      //change_possible
      i = bi.address % N;
      //shift new addr into history addr array
      shift_int(sv, (H_LEN + 1), i);
      //TODO
      //use memcpy to expedite
      for (j = 0; j <= H_LEN; j++){
        u.v[j] = sv[j];
        u.h[j] = sg[j];
      }

      y = w[i][0] + sr[H_LEN];

      prediction = (y >= 0);

      for (j = 1; j <= H_LEN; j++){
        k = H_LEN - j;
        //update sr vector
        if (prediction == TAKEN)
          sr[k + 1] = sr[k] + w[i][j];
        else
          sr[k + 1] = sr[k] - w[i][j];
      }
      sr[0] = 0;

      //shift new speculative into s history
      shift_bool(sg, (H_LEN + 1), prediction);

      u.i = i;
      u.y = y;
      u.direction_prediction(prediction);

      return &u;
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
    void update (branch_update *bu, bool taken, unsigned int target) {
      path_perceptron_update* u = (path_perceptron_update*)bu;
      //i: index modulo n, j: loop counter, k: index in r&w, y: y_out
      int i, j, k, y;

      i = u->i;
      y = u->y;

      //non-speculative r computation
      for (j = 1; j <= H_LEN; j++){
        k = H_LEN - j;

        if(taken == TAKEN)
          r[k + 1] = r[k] + w[i][j];
        else
          r[k + 1] = r[k] - w[i][j];
      }
      r[0] = 0;

      //shift actual branch result into non-speculative g
      shift_bool(g, (H_LEN + 1), taken);
      shift_int(v, (H_LEN + 1), i);

      // recover from misprediction, if any
      if (taken != u->direction_prediction()){
        for (j = 0; j <= H_LEN; j++){
          sr[j] = r[j];
          sg[j] = g[j];
          sv[j] = v[j];
        }
      }

      // perceptron learning rule
      if (taken != u->direction_prediction() || abs(y) < THETA){
        w[i][0] = satincdec(w[i][0], taken);
        for (j = 1; j <= H_LEN; j++){
          k = u->v[j];
          w[k][j] = satincdec(w[k][j], (taken == u->h[j]));
        }
      }

      //bool outcome = taken;
      //bool prediction = u->direction_prediction();

      //if ((prediction != outcome) || (this->abs(y_out) <= THETA)){
      //  w[i][0] = w[i][0] + (outcome == taken? 1 : -1);
      //  int j = 1;
      //  for (j = 1; j < HISTORY_LENGTH; j++){
      //    int k = v[j];
      //    w[k][j] = w[k][j] + (outcome == )
      //  }
      //}
    }

    unsigned int abs(int n){
      int ret[] = {n, -n};
      return ret[n < 0];
    }

    int satincdec (int w, bool inc) {
      if (inc) {
        if (w < max_weight) w++;
      }
      else {
        if (w > min_weight) w--;
      }
      return w;
    }

    //both shift functions are designed to shift the new value
    //into the fisrt(0) position of the array
    void shift_int(int array[], int length, int number){
      int j = 0;
      for (j = length - 1; j > 0; j--){
        array[j] = array[j - 1];
      }
      array[0] = number;
    }

    void shift_bool(bool array[], int length, int new_bool){
      int j = 0;
      for (j = length - 1; j > 0; j--){
        array[j] = array[j - 1];
      }
      array[0] = new_bool;
    }
};
