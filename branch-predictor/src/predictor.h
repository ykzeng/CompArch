// predictor.h
// This file declares branch_update and branch_predictor classes.

class branch_update {
	bool _direction_prediction;
	unsigned int _target_prediction;

public:
	bool direction_prediction () { return _direction_prediction; }
	void direction_prediction (bool b) { _direction_prediction = b; }

	bool target_prediction () { return _target_prediction; }
	void target_prediction (unsigned int t) { _target_prediction = t; }

	branch_update (void) : 
		_direction_prediction(false), _target_prediction(0) {}
};

class branch_predictor {
public:
	virtual branch_update *predict (branch_info &) = 0;
	virtual void update (branch_update *, bool, unsigned int) {}
	virtual ~branch_predictor (void) {}
};
