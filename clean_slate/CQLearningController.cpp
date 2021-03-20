/**
         (                                      
   (     )\ )                                   
 ( )\   (()/(   (    ) (        (        (  (   
 )((_)   /(_)) ))\( /( )(   (   )\  (    )\))(  
((_)_   (_))  /((_)(_)|()\  )\ |(_) )\ )((_))\  
 / _ \  | |  (_))((_)_ ((_)_(_/((_)_(_/( (()(_) 
| (_) | | |__/ -_) _` | '_| ' \)) | ' \)) _` |  
 \__\_\ |____\___\__,_|_| |_||_||_|_||_|\__, |  
                                        |___/   

Refer to Watkins, Christopher JCH, and Peter Dayan. "Q-learning." Machine learning 8. 3-4 (1992): 279-292
for a detailed discussion on Q Learning
*/
#include "CQLearningController.h"

#define DBOUT( s )            \
{                             \
   std::ostringstream os_;    \
   os_ << s;                   \
   OutputDebugString( os_.str().c_str() );  \
}

CQLearningController::CQLearningController(HWND hwndMain):
	CDiscController(hwndMain),
	_grid_size_x(CParams::WindowWidth / CParams::iGridCellDim + 1),
	_grid_size_y(CParams::WindowHeight / CParams::iGridCellDim + 1)
{
}
/**
 The update method should allocate a Q table for each sweeper (this can
 be allocated in one shot - use an offset to store the tables one after the other)

 You can also use a boost multiarray if you wish
*/
void CQLearningController::InitializeLearningAlgorithm(void)
{
	qTable.resize(400);
	for (int i = 0; i < 400; i++) {
		qTable[i].resize(400);
		for (int j = 0; j < 400; j++) {
			qTable[i][j].resize(4, 0);

		}
	}

}
/**);
 The immediate reward function. This computes a reward upon achieving the goal state of
 collecting all the mines on the field. It may also penalize movement to encourage exploring all directions and 
 of course for hitting supermines/rocks!
*/
double CQLearningController::R(uint x, uint y, uint sweeper_no) {
	//TODO: roll your own here
	SVector2D<int> posVector(x, y);
	bool isPickedUp = false;
	if (noMines == m_NumMines) {
		return 500;
	}
	for (int i = 0; i < m_vecObjects.size(); i++) {
		if (m_vecObjects[i]->getPosition().x == x && m_vecObjects[i]->getPosition().y == y) {
			isPickedUp = false;
			switch (m_vecObjects[i]->getType()) {

			case CCollisionObject::Mine:
				for each (SVector2D<int> vec in pickedUp)
				{
					if (vec.x == posVector.x && vec.y == posVector.y) {
						isPickedUp = true;
					}
				}
				if (!isPickedUp) {
					pickedUp.push_back(posVector);
					noMines += 1;
					return 1;
				}
				
				return -1;

			case CCollisionObject::SuperMine:
				return -100;

			case 2:
				throw "rock";
			}

		}

	}
	std::cout << "Empty" << endl;
	return -1;
}
/**
The update method. Main loop body of our Q Learning implementation
See: Watkins, Christopher JCH, and Peter Dayan. "Q-learning." Machine learning 8. 3-4 (1992): 279-292
*/
bool CQLearningController::Update(void)
{
	SVector2D<int> currentPos;
	SVector2D<int> prevPos;
	int action = 0;
	double reward = 0;
	//m_vecSweepers is the array of minesweepers
	//everything you need will be m_[something] ;)
	uint cDead = std::count_if(m_vecSweepers.begin(),
							   m_vecSweepers.end(),
						       [](CDiscMinesweeper * s)->bool{
								return s->isDead();
							   });
	if (cDead == CParams::iNumSweepers){
		qTable[prevPos.x][prevPos.y][action] = -100;
		pickedUp.clear();
		printf("All dead ... skipping to next iteration\n");
		noMines = 0;
		m_iTicks = CParams::iNumTicks;
	}

	// up = 0, right = 1, down = 2, left = 3
	for (uint sw = 0; sw < CParams::iNumSweepers; ++sw){
		if (m_vecSweepers[sw]->isDead()) {
			continue;
		}
		/**
		Q-learning algorithm according to:
		Watkins, Christopher JCH, and Peter Dayan. "Q-learning." Machine learning 8. 3-4 (1992): 279-292
		*/
		//1:::Observe the current state:	
		currentPos = m_vecSweepers[sw]->Position();

		//2:::Select action with highest historic return:
		action = ChooseAction(currentPos);

		m_vecSweepers[sw]->setRotation((ROTATION_DIRECTION)action);
		//now call the parents update, so all the sweepers fulfill their chosen action
	}
	
	CDiscController::Update(); //call the parent's class update. Do not delete this.
	if (m_iIterations == 51) {
		ofstream myfile;
		myfile.open("test.txt");
		int totalGathered = 0;
		int totalDeaths = 0;
		int mostGathered = 0;
		for (int i = 0; i < 50; i++) {
			myfile << "mines: " << m_vecMostMinesGathered[i] << " average mines: " << m_vecAvMinesGathered[i] << std::endl;
			totalGathered += m_vecMostMinesGathered[i];
			totalDeaths += m_vecDeaths[i];
			if (m_vecAvMinesGathered[i] > mostGathered) {
				mostGathered = m_vecAvMinesGathered[i];
			}
		}
		double avGathered = double(totalGathered) / 50;
		double avDeaths = double(totalDeaths) / 50;

		myfile << std::endl;
		myfile << "most mines gathered: " << mostGathered << " average mines gathered: " << avGathered << " average deaths: " << avDeaths;
		myfile.close();
	}
	for (uint sw = 0; sw < CParams::iNumSweepers; ++sw){

		currentPos = m_vecSweepers[sw]->Position();
		prevPos = m_vecSweepers[sw]->PrevPosition();

		if (m_vecSweepers[sw]->isDead()) {
			continue;
		}

		
		int nextAction = ChooseAction(currentPos);
		int xVal = currentPos.x;
		int yVal = currentPos.y;
		if (xVal >= 400) {
			xVal = 399;
		}
		if (yVal == 400) {
			yVal = 399;
		}
		double actionQ = qTable[xVal][yVal][nextAction];

		reward = R(xVal, yVal, sw);
		xVal = prevPos.x;
		yVal = prevPos.y;
		if (xVal == 400) {
			xVal = 399;
		}
		if (yVal == 400) {
			yVal = 399;
		}

		double newQ = qTable[xVal][yVal][action] +learningRate*(reward + discount*actionQ - qTable[xVal][yVal][action]);


		qTable[xVal][yVal][action] = newQ;

	}
	return true;
}

CQLearningController::~CQLearningController(void)
{
	//TODO: dealloc stuff here if you need to	
}

int  CQLearningController::ChooseAction(SVector2D<int> currentPos) {
	int xVal = currentPos.x;
	int yVal = currentPos.y;
	if (xVal == 400) {
		xVal = 399;
	}
	if (yVal == 400) {
		yVal = 399;
	}
	double up = qTable[xVal][yVal][0];
	double right = qTable[xVal][yVal][1];
	double down = qTable[xVal][yVal][2];
	double left = qTable[xVal][yVal][3];

	double maxQ = up;

	vector<int> possibleAction;

	bool isFound = false;
	for each (SVector2D<int> vec in pickedUp) {

		if (vec.x == xVal && vec.y == yVal) {
			isFound = true;
			break;
		}

	}
	if (!isFound) {

		possibleAction.push_back(0);
		
	}

	if (right == maxQ) {
		isFound = false;
		for each (SVector2D<int> vec in pickedUp) {

			if (vec.x == xVal && vec.y == yVal) {
				isFound = true;
				break;
			}

		}
		if (!isFound) {

			possibleAction.push_back(1);
		}
		
	}
	else if (right > maxQ) {
		isFound = false;
		for each (SVector2D<int> vec in pickedUp) {

			if (vec.x == xVal && vec.y == yVal) {
				isFound = true;
				break;
			}

		}
		if (!isFound) {

			possibleAction.clear();
			possibleAction.push_back(1);
			maxQ = right;
		}
	
		
	}

	if (down == maxQ) {
		isFound = false;
		for each (SVector2D<int> vec in pickedUp) {

			if (vec.x == xVal && vec.y == yVal) {
				isFound = true;
				break;
			}

		}
		if (!isFound) {

			possibleAction.push_back(2);
		}
	}
	else if (down > maxQ) {
		isFound = false;
		for each (SVector2D<int> vec in pickedUp) {

			if (vec.x == xVal && vec.y == yVal) {
				isFound = true;
				break;
			}

		}
		if (!isFound) {

			possibleAction.clear();
			possibleAction.push_back(2);
			maxQ = right;
			
		}
		
	}

	if (left == maxQ) {
		isFound = false;
		for each (SVector2D<int> vec in pickedUp) {

			if (vec.x == xVal && vec.y == yVal) {
				isFound = true;
				break;
			}

		}
		if (!isFound) {

			possibleAction.push_back(3);
		}
	}
	else if (left > maxQ) {
		isFound = false;
		for each (SVector2D<int> vec in pickedUp) {

			if (vec.x == xVal && vec.y == yVal) {
				isFound = true;
				break;
			}

		}
		if (!isFound) {

			possibleAction.clear();
			possibleAction.push_back(3);
			maxQ = left;
			

		}
	}
	int random = 0;
	int action = 0;
	if (possibleAction.size() != 0) {
		 random = rand() % possibleAction.size();
		 action = possibleAction[random];
	}
	
	return action;
	

}
