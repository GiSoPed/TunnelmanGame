#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
class StudentWorld;

// Students:  Add code to this file, Actor.cpp, StudentWorld.h, and StudentWorld.cpp
class Actor : public GraphObject
{
public:
	Actor(StudentWorld* wptr, int imageID, int sx, int sy, Direction sDirection, double size, unsigned int depth);
	virtual ~Actor();
	virtual void doSomething() = 0;
	StudentWorld* getWorld() const;
	int getX() const;
	int getY() const;
	void setX(int newX);
	void setY(int newY);
	void die();
	bool getLifeStat() const;
	int getTicks() const { return tickNum; }
	void incrementTicks() { tickNum++; }

private:
	StudentWorld* m_world;
	int x, y;
	bool lifeStat;
	int tickNum;
};
//--------------------------------------------------------

class Earth : public Actor
{
public:
	Earth(StudentWorld* wptr, int x, int y);
	virtual ~Earth();
	virtual void doSomething() {}
};
//--------------------------------------------------------

class Boulder : public Actor
{
public:
	Boulder(StudentWorld* wptr, int x, int y);
	virtual ~Boulder() {};
	virtual void doSomething();

private:
	char stability;
};
//--------------------------------------------------------

class Squirt : public Actor
{
public:
	Squirt(StudentWorld* wptr, int x, int y, Direction direct);
	virtual ~Squirt() {};
	virtual void doSomething();

private:
	int travelDis;
};
//--------------------------------------------------------
class HiddenItem : public Actor
{
public:
	HiddenItem(StudentWorld* wptr, int x, int y, int image);
	virtual ~HiddenItem() {};
	virtual void doSomething() = 0;

	bool getVis() const { return visability; }
	void setVis(bool v) { visability = v; }

private:
	bool visability;
};

//--------------------------------------------------------
class Barrel : public HiddenItem
{
public:
	Barrel(StudentWorld* wptr, int x, int y);
	virtual ~Barrel() {}
	virtual void doSomething();
};
//--------------------------------------------------------

class Gold : public HiddenItem
{
public:
	Gold(StudentWorld* wptr, int x, int y, bool drop);
	virtual ~Gold() {}
	virtual void doSomething();

private: 
	bool dropped;
	char permanence;
};

//--------------------------------------------------------

class PopUpItem : public Actor
{
public:
	PopUpItem(StudentWorld* wptr, int x, int y, int image);
	virtual ~PopUpItem() {};
	virtual void doSomething() = 0;
	int getT() const { return T; }

private:
	int T;
};

//--------------------------------------------------------

class SonarKit : public PopUpItem
{
public:
	SonarKit(StudentWorld* wptr, int x, int y);
	virtual ~SonarKit() {};
	virtual void doSomething();
};

//--------------------------------------------------------

class Water : public PopUpItem
{
public:
	Water(StudentWorld* wptr, int x, int y);
	virtual ~Water() {};
	virtual void doSomething();
};
//--------------------------------------------------------

class Person : public Actor
{
public:
	Person(StudentWorld* wptr, int image, int hp, Direction dir, int x, int y);
	virtual ~Person() {};
	virtual void doSomething() = 0;
	virtual void move(Direction direct) = 0;
	virtual void agitate(int num) = 0;

	int getHP() const;
	int getGold() const;
	void decHealthPoints(int points);
	void collectGold();
	void dropGold() { numGold--; }

private:
	int healthPoints, numGold;
};
//--------------------------------------------------------

class TunnelMan : public Person
{
public:
	TunnelMan(StudentWorld* wptr);
	virtual ~TunnelMan();
	virtual void doSomething();
	virtual void move(Direction direct);
	virtual void agitate(int num);

	void collectBarrel();
	void collectSonarKit();
	void collectWater();

	int getWater() const;
	int getSonar() const;
	int getBarrels() const;

private:
	int numWater, numSonar, numBarrels;
	void squirtWater();
};
//--------------------------------------------------------

class Protester : public Person
{
public:
	Protester(StudentWorld* wptr, int hp, int image);
	virtual ~Protester() {};
	virtual void doSomething();
	virtual void move(Direction direct);
	virtual void bribe() = 0;
	virtual void agitate(int num);

	void setN(int ticks);
	void stun() { stunned = true; }
	bool getStun() const { return stunned; }
	void leaveField();
	bool canMoveInDirection(Direction direct);

private:
	bool facingPlayer();
	char dirFacingPlayer();
	void switchDirRand(Direction cur);
	bool lineOfSight(Direction direct);
	bool atIntersection();
	bool canShout();
	bool canTurn();
	void turn();

	bool leavingField, stunned;
	int ticksToWait, N, tickNum, restTick, nonRestingCount, lastShout, lastTurn, numSquaresToMove;
};
//--------------------------------------------------------

class Regular : public Protester
{
public:
	Regular(StudentWorld* wptr);
	virtual ~Regular() {}
	virtual void bribe();
};
//--------------------------------------------------------

class Hardcore : public Protester
{
public:
	Hardcore(StudentWorld* wptr);
	virtual ~Hardcore() {}
	virtual void bribe();
};

#endif // ACTOR_H_
