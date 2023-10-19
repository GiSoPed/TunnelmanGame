#include "Actor.h"
#include "StudentWorld.h"
#include <vector>
using namespace std;

// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp

//-----------------------ACTOR---------------------------

Actor::Actor(StudentWorld* w_ptr, int imageID, int sx, int sy, Direction sDirection, double size, unsigned int depth)
	: GraphObject(imageID, sx, sy, sDirection, size, depth), m_world(w_ptr), tickNum(0)
{
	setVisible(true);
	x = sx;
	y = sy;
	lifeStat = true;
}

Actor::~Actor()
{
	m_world = nullptr;
	delete m_world;
}

StudentWorld* Actor::getWorld() const
{
	return m_world;
}

int Actor::getX() const
{
	return x;
}

int Actor::getY() const
{
	return y;
}

void Actor::setX(int newX)
{
	x = newX;
}

void Actor::setY(int newY)
{
	y = newY;
}

bool Actor::getLifeStat() const
{
	return lifeStat;
}

void Actor::die()
{
	lifeStat = false;
}

//-----------------------EARTH------------------------------
Earth::Earth(StudentWorld* wptr, int x, int y)
	: Actor(wptr, TID_EARTH, x, y, right, .25, 3)
{}

Earth::~Earth()
{}

//-----------------------SQUIRTS-----------------------------
Squirt::Squirt(StudentWorld* wptr, int x, int y, Direction direct)
	: Actor(wptr, TID_WATER_SPURT, x, y, direct, 1.0, 1), travelDis(4)
{}

void Squirt::doSomething()
{
	if (!getLifeStat())
		return;

	Protester* p = getWorld()->pInRadius(this, 3);
	if (p != nullptr ) 
	{
		die();
		if(!p->getStun())
			p->agitate(2);
		return;
	}

	if (travelDis == 0)
	{
		die();
		return;
	}

	switch (getDirection())
	{
	case right:
		if (getWorld()->overlapEarth(getX() + 1, getY()) 
			|| getWorld()->overlapBoulder(getX() + 1, getY(), 3) || p != nullptr) 
		{
			die(); 
			return;
		}
		moveTo(getX() + 1, getY());
		setX(getX() + 1);
		travelDis--;
		break;
	case left:
		if (getWorld()->overlapEarth(getX() - 1, getY())
			|| getWorld()->overlapBoulder(getX() - 1, getY(), 3))
		{
			die();
			return;
		}
		moveTo(getX() - 1, getY());
		setX(getX() - 1);
		travelDis--;
		break;
	case up:
		if (getWorld()->overlapEarth(getX(), getY() + 1)
			|| getWorld()->overlapBoulder(getX(), getY() + 1, 3))
		{
			die();
			return;
		}
		moveTo(getX(), getY() + 1);
		setY(getY() + 1);
		travelDis--;
		break;
	case down:
		if (getWorld()->overlapEarth(getX(), getY() - 1)
			|| getWorld()->overlapBoulder(getX(), getY() - 1, 3))
		{
			die();
			return;
		}
		moveTo(getX(), getY() - 1);
		setY(getY() - 1);
		travelDis--;
		break;
	default:
		return;
	}
}

//-----------------------BOULDER-----------------------------
Boulder::Boulder(StudentWorld* wptr, int x, int y): Actor(wptr, TID_BOULDER, x, y, down, 1.0, 1)
{
	stability = 'S';
}

void Boulder::doSomething()
{
	if (!getLifeStat())
		return;
	if (stability == 'S') 
	{
		if (!getWorld()->overlapEarth(getX(), getY()-1)) 
		{
			stability = 'W';
		}
	}
	else if (stability == 'W')
	{
		incrementTicks();
		if (getTicks() == 30)
		{
			stability = 'F';
			getWorld()->playSound(SOUND_FALLING_ROCK);
		}
	}
	else if (stability == 'F')
	{
		if (!getWorld()->overlapEarth(getX(), getY()-1) 
			&& !getWorld()->overlapBoulder(getX(), getY() - 1, 0))
		{
			moveTo(getX(), getY() - 1);
			setY(getY() - 1);
		}
		else if ((getWorld()->overlapEarth(getX(), getY()-1))
			|| (getWorld()->overlapBoulder(getX(), getY() - 1, 0))
			|| (getY() <= 3))
		{
			die();
		}

		int x = this->getX();
		int y = this->getY();
		int a = getWorld()->getTunnelMan()->getX();
		int b = getWorld()->getTunnelMan()->getY();

		if (getWorld()->inRadius(x, y, a, b, 3)) 
		{
			getWorld()->getTunnelMan()->agitate(100);
		}

		Protester* p = getWorld()->pInRadius(this, 3);
		if (p != nullptr) {
			die();
			p->agitate(100);
			getWorld()->increaseScore(500);
		}
	}
}

//-----------------------HIDDEN ITEM--------------------------

HiddenItem::HiddenItem(StudentWorld* wptr, int x, int y, int image): Actor(wptr, image, x, y, right, 1.0, 2)
{
	visability = false;
	setVisible(false);
}

//-----------------------OIL BARREL--------------------------

Barrel::Barrel(StudentWorld* wptr, int x, int y): HiddenItem(wptr, x, y, TID_BARREL)
{}

void Barrel::doSomething()
{
	if (!getLifeStat())
		return;
	if (!getVis()
		&& getWorld()->inRadius(getX(), getY(), getWorld()->getTunnelMan()->getX(), getWorld()->getTunnelMan()->getY(), 4))
	{
		setVisible(true);
		setVis(true);
		return;
	}
	else if (getWorld()->inRadius(getX(), getY(), getWorld()->getTunnelMan()->getX(), getWorld()->getTunnelMan()->getY(), 3))
	{
		die();
		getWorld()->playSound(SOUND_FOUND_OIL);
		getWorld()->increaseScore(1000);
		getWorld()->getTunnelMan()->collectBarrel();
	}
}

//-----------------------GOLD NUGGET------------------------

Gold::Gold(StudentWorld* wptr, int x, int y, bool drop) : HiddenItem(wptr, x, y, TID_GOLD)
{
	if (drop == true) 
	{
		dropped = true;
		setVis(true);
		setVisible(true);
		permanence = 'T';
		return;
	}
	else if (drop == false)
	{
		dropped = false;
		setVis(false);
		permanence = 'P';
		return;
	}
}

void Gold::doSomething()
{
	incrementTicks();
	if (!getLifeStat())
		return;
	if (!getVis()
		&& getWorld()->inRadius(getX(), getY(), getWorld()->getTunnelMan()->getX(), getWorld()->getTunnelMan()->getY(), 4))
	{
		setVisible(true);
		setVis(true);
		return;
	}
	else if (permanence == 'P' 
		&& getWorld()->inRadius(getX(), getY(), getWorld()->getTunnelMan()->getX(), getWorld()->getTunnelMan()->getY(), 3))
	{
		die();
		getWorld()->playSound(SOUND_GOT_GOODIE);
		getWorld()->increaseScore(10);
		getWorld()->getTunnelMan()->collectGold();
		return;
	}
	else if (permanence == 'T')
	{
		if (getTicks() > 100)
		{
			die();
			return;
		}

		Protester* p = (getWorld()->pInRadius(this, 3));
		if (p != nullptr && !p->getStun()) 
		{
			die();
			p->bribe();
		}

	}
}

//-----------------------PICK-UP ITEM-----------------------

PopUpItem::PopUpItem(StudentWorld* wptr, int x, int y, int image)
	: Actor(wptr, image, x, y, right, 1.0, 2)
{
	T = max(100, 300 - 10 * (int)getWorld()->getLevel());
}

//-----------------------SONAR KIT--------------------------

SonarKit::SonarKit(StudentWorld* wptr, int x, int y): PopUpItem(wptr, x, y, TID_SONAR)
{}

void SonarKit::doSomething()
{

	if (!getLifeStat())
		return;

	if (getTicks() >= getT())
	{
		die();
	}

	if (getWorld()->inRadius(getX(), getY(), getWorld()->getTunnelMan()->getX(), getWorld()->getTunnelMan()->getY(), 3))
	{
		die();
		getWorld()->playSound(SOUND_GOT_GOODIE);
		getWorld()->increaseScore(75);
		getWorld()->getTunnelMan()->collectSonarKit();
	}

	incrementTicks();
}

//-----------------------WATER POOL--------------------------

Water::Water(StudentWorld* wptr, int x, int y) : PopUpItem(wptr, x, y, TID_WATER_POOL)
{}

void Water::doSomething()
{
	if (!getLifeStat())
		return;

	if (getTicks() >= getT())
	{
		die();
	}

	if (getWorld()->inRadius(getX(), getY(), getWorld()->getTunnelMan()->getX(), getWorld()->getTunnelMan()->getY(), 3))
	{
		die();
		getWorld()->playSound(SOUND_GOT_GOODIE);
		getWorld()->increaseScore(100);
		getWorld()->getTunnelMan()->collectWater();
	}

	incrementTicks();
}

//-----------------------PERSON------------------------------

Person::Person(StudentWorld* wptr, int image, int hp, Direction dir, int x, int y)
	: Actor(wptr, image, x, y, dir, 1.0, 0), healthPoints(hp), numGold(0)
{}

int Person::getHP() const
{
	return healthPoints;
}

void Person::decHealthPoints(int points)
{
	healthPoints -= points;
}

void Person::collectGold()
{
	numGold++;
}

int Person::getGold() const
{
	return numGold;
}

//-----------------------TUNNELMAN---------------------------
TunnelMan::TunnelMan(StudentWorld* wptr)
	: Person(wptr, TID_PLAYER, 10, right, 30, 60),
	numWater(5), numSonar(1), numBarrels(0)
{}

TunnelMan::~TunnelMan()
{}

void TunnelMan::squirtWater()
{
	switch (getDirection()) {
	case right:
		if(!getWorld()->overlapEarth(getX() + 4, getY()) 
			&& !getWorld()->overlapBoulder(getX() + 4, getY(), 3))
			getWorld()->addActor(new Squirt(getWorld(), getX() + 4, getY(), getDirection()));
		break;
	case left:
		if (!getWorld()->overlapEarth(getX() - 4, getY())
			&& !getWorld()->overlapBoulder(getX() - 4, getY(), 3))
			getWorld()->addActor(new Squirt(getWorld(), getX() - 4, getY(), getDirection()));
		break;
	case up:
		if (!getWorld()->overlapEarth(getX(), getY() + 4)
			&& !getWorld()->overlapBoulder(getX(), getY() + 4, 3))
			getWorld()->addActor(new Squirt(getWorld(), getX(), getY() + 4, getDirection()));
		break;
	case down:
		if (!getWorld()->overlapEarth(getX(), getY() - 4)
			&& !getWorld()->overlapBoulder(getX(), getY() - 4, 3))
			getWorld()->addActor(new Squirt(getWorld(), getX(), getY() - 4, getDirection()));
		break;
	}
}

void TunnelMan::move(Direction direct)
{
	//If not already facing direction then change direction otherwise move in direction
	if (getDirection() != direct)
	{
		setDirection(direct);
		return;
	}
	else
	{
		switch (direct)
		{
		case right:
			if (!getWorld()->overlapBoulder(getX() + 1, getY(), 4) && getX() < VIEW_WIDTH - 4)
			{
				moveTo(getX() + 1, getY());
				setX(getX() + 1);
			}
			break;
		case left:
			if (!getWorld()->overlapBoulder(getX() - 1, getY(), 4) && getX() > 0)
			{
				moveTo(getX() - 1, getY());
				setX(getX() - 1);
			}
			break;
		case up:
			if (!getWorld()->overlapBoulder(getX(), getY() + 1, 4) && getY() < VIEW_HEIGHT - 4)
			{
				moveTo(getX(), getY() + 1);
				setY(getY() + 1);
			}
			break;
		case down:
			if (!getWorld()->overlapBoulder(getX(), getY() - 1, 4) && getY() > 0)
			{
				moveTo(getX(), getY() - 1);
				setY(getY() - 1);
			}
			break;
		default:
			return;
		}
	}
}

void TunnelMan::doSomething()
{
	//Check to see that the player is still alive
	if (!getLifeStat())
		return;

	//If player overlaps with Earth, delete Earth
	if (getWorld()->overlapEarth(getX(), getY()))
	{
		getWorld()->dig(getX(), getY());
		getWorld()->playSound(SOUND_DIG);
	}

	//If player presses a key execute action
	int key;
	if (getWorld()->getKey(key))
	{
		switch (key)
		{
		case KEY_PRESS_ESCAPE:	//Escape = End Game
			die();
			break;
		case KEY_PRESS_SPACE:	//Space Bar = Squirt into oil field
			if (numWater > 0)
			{
				squirtWater();
				getWorld()->playSound(SOUND_PLAYER_SQUIRT);
				numWater--;
			}
			break;
		case KEY_PRESS_UP:
			move(up);
			break;
		case KEY_PRESS_DOWN:
			move(down);
			break;
		case KEY_PRESS_RIGHT:
			move(right);
			break;
		case KEY_PRESS_LEFT:
			move(left);
			break;
		case 'Z':
		case 'z':
			if (numSonar > 0) 
			{
				getWorld()->sonarScan(getX(), getY());
				numSonar--;
			}
			else return;
			break;
		case KEY_PRESS_TAB:
			if (getGold() > 0)
			{
				getWorld()->addActor(new Gold(getWorld(), getX(), getY(), true));
				dropGold();
			}
			break;
		default:
			return;
		}
	}
}

void TunnelMan::agitate(int num)
{
	decHealthPoints(num);
	if (getHP() <= 0) {
		die();
		getWorld()->playSound(SOUND_PLAYER_GIVE_UP);
	}
}

void TunnelMan::collectBarrel() 
{
	numBarrels++;
}

void TunnelMan::collectSonarKit()
{
	numSonar++;
}

void TunnelMan::collectWater()
{
	numWater++;
}

int TunnelMan::getWater() const
{
	return numWater;
}

int TunnelMan::getSonar() const
{
	return numSonar;
}

int TunnelMan::getBarrels() const
{
	return numBarrels;
}

//-----------------------PROTESTER---------------------------

Protester::Protester(StudentWorld* wptr, int hp, int image)
	: Person(wptr, image, hp, left, 60, 60), leavingField(false), stunned(false),
	nonRestingCount(0), tickNum(0), lastShout(0), lastTurn(0)
{
	ticksToWait = max(0, 3 - (int)getWorld()->getLevel() / 4);
	numSquaresToMove = rand() % 53 + 8;
	N = max(50, 100 - (int)getWorld()->getLevel() * 10);
	restTick = 0;
}

void Protester::setN(int ticks)
{
	N = ticks;
}

void Protester::agitate(int num)
{
	if (leavingField != true)
	{
		decHealthPoints(num);
		getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
		if (getHP() <= 0) {
			leavingField = true;
			getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
			if (stunned)
				stunned = false;
		}
		else
			stunned = true;
	}
}

void Protester::leaveField()
{
	leavingField = true;
}

bool Protester::canShout()
{
	if (lastShout + 15 <= nonRestingCount || lastShout == 0)
		return true;
	else
		return false;
}

bool Protester::canTurn()
{
	if (lastTurn + 200 <= nonRestingCount || lastTurn == 0)
	{
		lastTurn = nonRestingCount;
		return true;
	}	
	else
		return false;
}

void Protester::turn()
{
	Direction cur = getDirection();
	switch (cur)
	{
	case right:
		if (canMoveInDirection(up))
			setDirection(up);
		else if (canMoveInDirection(down))
			setDirection(down);
		else
			setDirection(left);
		break;
	case left:
		if (canMoveInDirection(up))
			setDirection(up);
		else if (canMoveInDirection(down))
			setDirection(down);
		else
			setDirection(right);
		break;
	case up:
		if (canMoveInDirection(right))
			setDirection(right);
		else if (canMoveInDirection(left))
			setDirection(left);
		else
			setDirection(down);
		break;
	case down:
		if (canMoveInDirection(right))
			setDirection(right);
		else if (canMoveInDirection(left))
			setDirection(left);
		else
			setDirection(up);
		break;
	case none:
		break;
	}
	return;
}

void Protester::move(Direction direct)
{
	switch (direct) {
	case right:
		if (getDirection() == right && canMoveInDirection(right))
		{
			if (getX() >= 60)
			{
				setDirection(left);
			}
			else
			{
				moveTo(getX() + 1, getY());
				setX(getX() + 1);
			}
		}
		else setDirection(right);
		break;
	case left:
		if (getDirection() == left && canMoveInDirection(left))
		{
			if (getX() <= 0)
			{
				setDirection(right);
			}
			else
			{
				moveTo(getX() - 1, getY());
				setX(getX() - 1);
			}
		}
		else setDirection(left);
		break;
	case up:
		if (getDirection() == up && canMoveInDirection(up))
		{
			if (getY() >= 60)
			{
				setDirection(down);
			}
			else
			{
				moveTo(getX(), getY() + 1);
				setY(getY() + 1);
			}
		}
		else setDirection(up);
		break;
	case down:
		if (getDirection() == down && canMoveInDirection(down))
		{
			if (getY() <= 0)
			{
				setDirection(up);
			}
			else
			{
				moveTo(getX(), getY() - 1);
				setY(getY() - 1);
			}
		}
		else setDirection(down);
		break;
	default:
		return;
	}
}

bool Protester::canMoveInDirection(Direction direct)
{
	int x = getX();
	int y = getY();
	switch (direct)
	{
	case right:
		if (getWorld()->overlapEarth(x + 1, y) || getWorld()->overlapBoulder(x + 1, y, 4) || x + 1 < 0)
			return false;
		else
			return true;
		break;
	case left:
		if (getWorld()->overlapEarth(x - 1, y) || getWorld()->overlapBoulder(x - 1, y, 4) || x - 1 > 60)
			return false;
		else
			return true;
		break;
	case up:
		if (getWorld()->overlapEarth(x, y + 1) || getWorld()->overlapBoulder(x, y + 1, 4) || y + 1 > 60)
			return false;
		else
			return true;
		break;
	case down:
		if (getWorld()->overlapEarth(x, y - 1) || getWorld()->overlapBoulder(x, y - 1, 4) || y - 1 < 0)
			return false;
		else
			return true;
		break;
	case none:
		return false;
	}
	
}

bool Protester::facingPlayer()
{
	Direction proDir = getDirection();
	int tx = getWorld()->getTunnelMan()->getX();
	int ty = getWorld()->getTunnelMan()->getY();
	int px = getX();
	int py = getY();

	switch (proDir)
	{
	case right:
		if (tx > px && ty == py)
			return true;
		else
			return false;
		break;
	case left:
		if(tx < px && ty == py)
			return true;
		else
			return false;
		break;
	case up:
		if (tx == px && ty > py)
			return true;
		else
			return false;
		break;
	case down:
		if (tx == px && ty < py)
			return true;
		else
			return false;
		break;
	case none:
		return false;
	}
}

char Protester::dirFacingPlayer()
{
	Direction proDir = getDirection();
	int tx = getWorld()->getTunnelMan()->getX();
	int ty = getWorld()->getTunnelMan()->getY();
	int px = getX();
	int py = getY();

	if (tx > px && ty == py)
		return 'R';
	if (tx < px && ty == py)
		return 'L';
	if (tx == px && ty > py)
		return 'U';
	if (tx == px && ty < py)
		return 'D';
}

void Protester::switchDirRand(Direction cur)
{
	bool pickedNewDir = false;
	do {
		int val = rand() % 4 + 1;
		switch (val)
		{
		case 1:
			if (canMoveInDirection(right))
			{
				setDirection(right);
				pickedNewDir = true;
				break;
			}
		case 2:
			if (canMoveInDirection(left))
			{
				setDirection(left);
				pickedNewDir = true;
				break;
			}
		case 3:
			if (canMoveInDirection(up))
			{
				setDirection(up);
				pickedNewDir = true;
				break;
			}
		case 4:
			if (canMoveInDirection(down))
			{
				setDirection(down);
				pickedNewDir = true;
				break;
			}
		}
	} while (!pickedNewDir);
	return;
}

bool Protester::lineOfSight(Direction direct)
{
	int tx = getWorld()->getTunnelMan()->getX();
	int ty = getWorld()->getTunnelMan()->getX();
	int px = getX();
	int py = getY();

	switch (direct) 
	{
	case right:
		for (int i = px; i <= tx; i++){
			if (getWorld()->overlapEarth(i, py) || getWorld()->overlapBoulder(i, py, 3))
				return false;
		}
		return true;
		break;
	case left:
		for (int i = px; i >= tx; i--){
			if (getWorld()->overlapEarth(i, py) || getWorld()->overlapBoulder(i, py, 3))
				return false;
		}
		return true;
		break;
	case up:
		for (int j = py; j <= ty; j++) {
			if (getWorld()->overlapEarth(px, j) || getWorld()->overlapBoulder(px, j, 3))
				return false;
		}
		return true;
		break;
	case down:
		for (int j = py; j >= ty; j--) {
			if (getWorld()->overlapEarth(px, j) || getWorld()->overlapBoulder(px, j, 3))
				return false;
		}
		return true;
		break;
	case none:
		return false;
	}

	return false;
}

bool Protester::atIntersection()
{
	if (tickNum == 0)
		return false;
	
	if (getDirection() == right || getDirection() == left)
	{
		return (canMoveInDirection(up) || canMoveInDirection(down));
	}
	else if (getDirection() == up || getDirection() == down)
	{
		return (canMoveInDirection(right) || canMoveInDirection(down));
	}

	return false;
}

void Protester::doSomething()
{
	if (!getLifeStat())
		return;

	if (getHP() == 0 && leavingField != true)
	{
		leavingField = true;
		getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
	}

	if (stunned == true && restTick < N)
	{
		restTick++;
		return;
	}
	else if (restTick >= N) 
	{
		restTick = 0;
		stunned = false;
	}

	if (tickNum % ticksToWait == 0)
	{
		if (leavingField == true)
		{

   			if (getX() == 60 && getY() == 60)
			{
				die();
				return;
			}
			else {
				Direction direct = getWorld()->exitMaze(this);
				if (direct != none)
				{
					move(direct);
				}
				tickNum++;
				return;
			}
		}
		else
		{
			nonRestingCount++;

			Direction protesterDir = getDirection();
			Direction playerDir = none;

			switch (dirFacingPlayer()) {
			case 'R': playerDir = right; break;
			case 'L': playerDir = left; break;
			case 'U': playerDir = up; break;
			case 'D': playerDir = down; break;
			default: playerDir = none; break;
			}

			if (getWorld()->inRadius(getX(), getY(), getWorld()->getTunnelMan()->getX(), getWorld()->getTunnelMan()->getY(), 4)
				&& facingPlayer() && canShout())
			{
				getWorld()->playSound(SOUND_PROTESTER_YELL);
				lastShout = nonRestingCount;
				getWorld()->getTunnelMan()->agitate(2);
				tickNum++;
				return;
			}

			if (getWorld()->inRadius(getX(), getY(), getWorld()->getTunnelMan()->getX(), getWorld()->getTunnelMan()->getY(), 4))
			{
				tickNum++;
				return;
			}

			if (getID() != TID_PROTESTER) {
				int M = 16 + int(getWorld()->getLevel()) * 2;
				Direction direct = getWorld()->detectTunnelMan(this, M);
				if (direct != none) 
				{
					move(direct);
					tickNum++;
					return;
				}
			}

			if (!getWorld()->inRadius(getX(), getY(), getWorld()->getTunnelMan()->getX(), getWorld()->getTunnelMan()->getY(), 4)
				&& canMoveInDirection(playerDir) && lineOfSight(playerDir))
			{
				setDirection(playerDir);
				move(playerDir);
				numSquaresToMove = 0;
				tickNum++;
				return;
			}

			if (numSquaresToMove <= 0)
			{
				switchDirRand(getDirection());
				numSquaresToMove = rand() % 53 + 8;
			}
			else if (atIntersection() && canTurn())
			{
				turn();
				numSquaresToMove = rand() % 53 + 8;
			}

			move(getDirection());
			numSquaresToMove--;
			if (!canMoveInDirection(getDirection())) {
				numSquaresToMove = 0;
			}
		}
	}
	tickNum++;
}

//----------------------REGULAR PROTESTER--------------------

Regular::Regular(StudentWorld* wptr): Protester(wptr, 5, TID_PROTESTER)
{}

void Regular::bribe()
{
	getWorld()->increaseScore(25);
	leaveField();
	getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
}

//----------------------HARDCORE PROTESTER--------------------

Hardcore::Hardcore(StudentWorld* wptr) : Protester(wptr, 20, TID_HARD_CORE_PROTESTER)
{}

void Hardcore::bribe()
{
	getWorld()->increaseScore(50);
	int t = max(50, 100 - int(getWorld()->getLevel()) * 10);
	setN(t);
	stun();
	getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
}