#pragma once
#include "cdisccontroller.h"
#include "CParams.h"
#include "CDiscCollisionObject.h"
#include <cmath>

typedef unsigned int uint;
class CQLearningController :
	public CDiscController
{
private:
	uint _grid_size_x;
	uint _grid_size_y;

	float learningRate = 0.1f;
	float discount = 0.9f;

	bool startState = true;

	int noMines = 0;
public:
	CQLearningController(HWND hwndMain);
	virtual void InitializeLearningAlgorithm(void);
	double R(uint x, uint y, uint sweeper_no);
	virtual bool Update(void);
	virtual ~CQLearningController(void);
	vector<vector<vector<double> > > qTable;
	vector<SVector2D<int> > pickedUp;
	int ChooseAction(SVector2D<int> currentPos);
};

