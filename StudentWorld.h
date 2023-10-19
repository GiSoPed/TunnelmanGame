#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "GraphObject.h"
#include <string>
#include <vector>
#include <queue>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp
class Actor;
class Protester;
class TunnelMan;
class Earth;

class StudentWorld : public GameWorld
{
public:
	StudentWorld(std::string assetDir)
		: GameWorld(assetDir), tickNum(0)
	{}
	virtual ~StudentWorld();
	virtual int init();
	virtual int move();
	virtual void cleanUp();

	void addActor(Actor* newActor);
	void dig(int x, int y); //dig up Earth
	bool overlapEarth(int x, int y);
	bool overlapBoulder(int x, int y, int r);
	TunnelMan* getTunnelMan() const;
	GraphObject::Direction exitMaze(Protester* pro);
	double distance(int x1, int x2, int y1, int y2);
	bool inRadius(int x, int y, int a, int b, int r);
	bool ActorsInRadius(int x, int y, int r);
	Protester* pInRadius(Actor* a, int r);
	GraphObject::Direction detectTunnelMan(Protester* pro, int M);
	void sonarScan(int x, int y);

private:
	void updateDisplayText();
	void generateBGL(int n, char item); //generate the Boulders, Gold, and Oil Barrels in the field

	std::vector<Actor*> actors;
	std::vector<Protester*> protesters;
	TunnelMan* player;
	Earth* field[64][64];
	int barrelMax;
	int tickNum, lastProtester, proCount;
	int T, P;

	int protesterMaze[64][64];
	struct Maze {
		int x;
		int y;
		Maze(int a, int b) : x(a), y(b) {}
	};

	bool bfs(std::queue<Maze*>& m, std::vector<Maze*>& traversal, int ex, int ey);
};
#endif // STUDENTWORLD_H_
