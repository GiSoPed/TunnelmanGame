#include "StudentWorld.h"
#include "Actor.h"
#include "GraphObject.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <queue>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::~StudentWorld()
{}

int StudentWorld::init()
{
	int B = min((int)getLevel() / 2 + 2, 9);
	barrelMax = min(2 + (int)getLevel(), 21);
	int G = max((int)getLevel() / 2, 2);
	T = max(25, 200 - (int)getLevel());
	P = min(15, 2 + (int)(getLevel() * 1.5));
	lastProtester = 1;
	proCount = 0;
	tickNum = 0;

	for (int i = 0; i < 64; i++)
	{
		for (int j = 0; j < 60; j++)
		{
			if ((i < 30 || i > 33) || (j < 4))
			{
				field[i][j] = new Earth(this, i, j);
			}
		}
	}

	player = new TunnelMan(this);

	int probabilityOfHardcore = min(90, (int)getLevel() * 10 + 30);
	if ( rand() % 100 > probabilityOfHardcore) {
		Regular* newProtester = new Regular(this);
		actors.push_back(newProtester);
		protesters.push_back(newProtester);
	}
	else {
		Hardcore* newProtester = new Hardcore(this);
		actors.push_back(newProtester);
		protesters.push_back(newProtester);
	}
	proCount = 1;

	generateBGL(B, 'B');
	generateBGL(barrelMax, 'L');
	generateBGL(G, 'G');

	return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
	// This code is here merely to allow the game to build, run, and terminate after you hit enter a few times.
	// Notice that the return value GWSTATUS_PLAYER_DIED will cause our framework to end the current level.
	updateDisplayText();

	vector<Actor*>::iterator it = actors.begin();
	while (it != actors.end()) {
		if ((*it)->getLifeStat()) {
			(*it)->doSomething();
		}
		if (!player->getLifeStat()) {
			decLives();
			return GWSTATUS_PLAYER_DIED;
		}
		else if (barrelMax - player->getBarrels() <= 0) {
			playSound(SOUND_FINISHED_LEVEL);
			return GWSTATUS_FINISHED_LEVEL;
		}
		it++;
	}
	player->doSomething();
	tickNum++;

	int x, y;
	int oddsOfItem = (int)getLevel() * 25 + 300;
	if (rand() % oddsOfItem == 0)
	{
		int waterProbability = rand() % 5;
		if (waterProbability == 0)
			addActor(new SonarKit(this, 0, 60));
		else {
			do {
				x = rand() % 60 + 1;
				y = rand() % 60 + 1;
			} while (overlapEarth(x, y) || overlapBoulder(x, y, 4));
			addActor(new Water(this, x, y));
		}
	}

	if (lastProtester + T <= tickNum && proCount < P) { //if conditions met then add protester
		int probabilityOfHardcore = min(90, (int)getLevel() * 10 + 30);
		if (rand() % 100 > probabilityOfHardcore) {
			Regular* newProtester = new Regular(this);
			actors.push_back(newProtester);
			protesters.push_back(newProtester);
		}
		else {
			Hardcore* newProtester = new Hardcore(this);
			actors.push_back(newProtester);
			protesters.push_back(newProtester);
		}
		proCount++;

		//reset ticks to new protester
		T = max(200 - (int)getLevel(), 25);
		lastProtester = tickNum;
	}

	if (!player->getLifeStat()) {
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}

	it = actors.begin();
	while (it != actors.end()) {
		if (!(*it)->getLifeStat()) {
			if ((*it)->getID() == TID_PROTESTER || (*it)->getID() == TID_HARD_CORE_PROTESTER)
			{
				vector<Protester*>::iterator it2 = protesters.begin();
				while (it2 != protesters.end()) 
				{
					if (!(*it)->getLifeStat())
						it2 = protesters.erase(it2);
				}
				proCount--;
			}
			delete* it;
			it = actors.erase(it);
		}
		else it++;
	}

	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
	for (int x = 0; x < 64; x++)
	{
		for (int y = 0; y < 60; y++)
		{
			delete field[x][y];
		}
	}

	for (vector<Actor*>::iterator it = actors.begin(); it != actors.end(); it++)
		delete* it;
	actors.clear();
	protesters.clear();

	delete player;
}

void StudentWorld::addActor(Actor* newActor)
{
	actors.push_back(newActor);
}

void StudentWorld::updateDisplayText()
{
	int score = getScore();
	int level = getLevel();
	int lives = getLives();
	int health = player->getHP();
	int squirts = player->getWater();
	int gold = player->getGold();
	int sonar = player->getSonar();
	int barrels = barrelMax - player->getBarrels();

	stringstream displayStr;
	displayStr.fill('0');
	displayStr << "Scr: " << setw(6) << score;
	
	displayStr.fill(' ');
	displayStr << "  Lvl: " << setw(2) << level
		<< "  Lives: " << setw(1) << lives
		<< "  Hlth: " << setw(3) << health * 10 << "%"
		<< "  Wtr: " << setw(2) << squirts
		<< "  Gld: " << setw(2) << gold
		<< "  Sonar: " << setw(2) << sonar
		<< "  Oil Left: " << setw(2) << barrels;

	string s = displayStr.str();
	setGameStatText(s);
}

bool StudentWorld::overlapEarth(int x, int y)
{
	bool returnVal = false;
	for (int i = x; i <= x + 3; i++)
	{
		for (int j = y; j <= y + 3; j++)
		{
			if (field[i][j] != nullptr)
				returnVal = true;
		}
	}
	return returnVal;
}

bool StudentWorld::overlapBoulder(int x, int y, int r)
{
	vector<Actor*>::iterator it;
	for (it = actors.begin(); it != actors.end(); it++) {
		if ((*it)->getID() == TID_BOULDER 
			&& inRadius(x, y, (*it)->getX(), (*it)->getY(), r))
			return true;
	}
	return false;
}

// redo
GraphObject::Direction StudentWorld::detectTunnelMan(Protester* pro, int M)
{
	for (int i = 0; i < 64; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			protesterMaze[i][j] = 0;
		}
	}

	queue<Maze*> m;
	vector<Maze*> path;
	m.push(new Maze(player->getX(), player->getY()));
	protesterMaze[player->getX()][player->getY()] = 1;
	bool b = bfs(m, path, pro->getX(), pro->getY());

	if (protesterMaze[pro->getX()][pro->getY()] - 1 <= M && b)
	{
		int a = pro->getX();
		int b = pro->getY();
		if (pro->canMoveInDirection(GraphObject::right) && protesterMaze[a + 1][b] < protesterMaze[a][b] && protesterMaze[a + 1][b] > 0)
			return GraphObject::right;
		if (pro->canMoveInDirection(GraphObject::left) && protesterMaze[a - 1][b] < protesterMaze[a][b] && protesterMaze[a - 1][b] > 0)
			return GraphObject::left;
		if (pro->canMoveInDirection(GraphObject::up) && protesterMaze[a][b + 1] < protesterMaze[a][b] && protesterMaze[a][b + 1] > 0)
			return GraphObject::up;
		if (pro->canMoveInDirection(GraphObject::down) && protesterMaze[a][b - 1] < protesterMaze[a][b] && protesterMaze[a][b - 1] > 0)
			return GraphObject::down;
	}
	else
		return GraphObject::none;
}

bool StudentWorld::bfs(queue<Maze*>& m, vector<Maze*>& traversal, int ex, int ey)
  {
	while (!m.empty())
	{
		Maze* current = m.front();
		m.pop();
		int sx = current->x;
		int sy = current->y;
		traversal.push_back(current);

		if (sx == ex && sy == ey)
		{
			return true;
		}

		if (!overlapEarth(sx + 1, sy) && !overlapBoulder(sx + 1, sy, 3) && protesterMaze[sx + 1][sy] == 0 && sx + 1 < 60) {
			m.push(new Maze(sx + 1, sy));
			protesterMaze[sx + 1][sy] = protesterMaze[sx][sy] + 1;
		}
		if (!overlapEarth(sx - 1, sy) && !overlapBoulder(sx - 1, sy, 3) && protesterMaze[sx - 1][sy] == 0 && sx - 1 > 0){
			m.push(new Maze(sx - 1, sy));
			protesterMaze[sx - 1][sy] = protesterMaze[sx][sy] + 1;
		}
		if (!overlapEarth(sx, sy + 1) && !overlapBoulder(sx, sy + 1, 3) && protesterMaze[sx][sy + 1] == 0 && sy + 1 < 60){
			m.push(new Maze(sx, sy + 1));
			protesterMaze[sx][sy + 1] = protesterMaze[sx][sy] + 1;
		}
		if (!overlapEarth(sx, sy - 1) && !overlapBoulder(sx, sy - 1, 3) && protesterMaze[sx][sy - 1] == 0 && sy - 1 > 0) {
			m.push(new Maze(sx, sy - 1));
			protesterMaze[sx][sy - 1] = protesterMaze[sx][sy] + 1;
		}
	}

	return false;
}

void StudentWorld::sonarScan(int x, int y)
{
	playSound(SOUND_SONAR);
	int n = actors.size();
	for (int i = 0; i < n; i++) 
	{
		if (distance(player->getX(), actors[i]->getX(), player->getY(), actors[i]->getY()) < 12) 
		{
			actors[i]->setVisible(true);
		}
	}
}

TunnelMan* StudentWorld::getTunnelMan() const
{
	return player;
}

void StudentWorld::dig(int x, int y)
{
	for (int i = x; i <= x + 3; i++)
	{
		for (int j = y; j <= y + 3; j++)
		{
			if (field[i][j] != nullptr) {
				delete field[i][j];
				field[i][j] = nullptr;
			}
		}
	}
}

double StudentWorld::distance(int x1, int x2, int y1, int y2) {
	double dist = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
	return dist;
}

bool StudentWorld::inRadius(int x, int y, int a, int b, int r) 
{
	if (sqrt((x - a) * (x - a) + (y - b) * (y - b)) < r)
		return true;
	else return false;
}

Protester* StudentWorld::pInRadius(Actor* a, int r)
{
	vector<Protester*>::iterator it = protesters.begin();
	while(it != protesters.end())
	{
		int dist = distance(a->getX(), (*it)->getX(), a->getY(), (*it)->getY());
		if (dist <= 3)
			return (*it);
		it++;
	}

	return nullptr;
}

bool StudentWorld::ActorsInRadius(int x, int y, int r) {
	bool returnVal = false;

	vector<Actor*>::iterator it = actors.begin();
	while (it != actors.end()) 
	{
		if (inRadius(x, y, (*it)->getX(), (*it)->getY(), r))
			returnVal = true;
		it++;
	}

	return returnVal;
}

void StudentWorld::generateBGL(int n, char itemType) 
{
	int x;
	int y;

	switch (itemType)
	{
	case 'B':
		for (int i = 0; i < n; i++)
		{
			do {
				x = rand() % 60 + 1;
				y = rand() % 35 + 21;
			} while ((x > 25 && x < 35 && y > 0) || ActorsInRadius(x, y, 6));
			actors.push_back(new Boulder(this, x, y));
			for (int i = x; i <= x + 3; i++)
			{
				for (int j = y; j <= y + 3; j++)
				{
					if (field[i][j] != nullptr) {
						delete field[i][j];
						field[i][j] = nullptr;
					}
				}
			}
		}
		break;
	case 'G':
		for (int r = 1; r <= n; r++)
		{
			do {
				x = rand() % 60 + 1;
				y = rand() % 56 + 1;
			} while ((x < 34 && x > 26 && y > 0) 
				|| ActorsInRadius(x, y, 6) || overlapBoulder(x, y, 4));
			actors.push_back(new Gold(this, x, y, false));
		}
		break;
	case 'L':
		for (int k = 1; k <= n; k++)
		{
			do {
				x = rand() % 60 + 1;
				y = rand() % 56 + 1;
			} while ((x < 34 && x > 26 && y > 0) 
				|| ActorsInRadius(x, y, 6) || overlapBoulder(x, y, 4));
			actors.push_back(new Barrel(this, x, y));
		}
		break;
	}
}

// redo
GraphObject::Direction StudentWorld::exitMaze(Protester* pro)
{
	for (int i = 0; i < 64; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			protesterMaze[i][j] = 0;

		}
	}

	queue<Maze*> m;
	vector<Maze*> path;
	m.push(new Maze(60, 60));
	protesterMaze[60][60] = 1;
	bool b = bfs(m, path, pro->getX(), pro->getY());

	if (b)
	{
		int bx = pro->getX();
		int by = pro->getY();
		if (pro->canMoveInDirection(GraphObject::right) && protesterMaze[bx + 1][by] < protesterMaze[bx][by] && protesterMaze[bx + 1][by] > 0)
			return GraphObject::right;
		if (pro->canMoveInDirection(GraphObject::left) && protesterMaze[bx - 1][by] < protesterMaze[bx][by] && protesterMaze[bx - 1][by] > 0)
			return GraphObject::left;
		if (pro->canMoveInDirection(GraphObject::up) && protesterMaze[bx][by + 1] < protesterMaze[bx][by] && protesterMaze[bx][by + 1] > 0)
			return GraphObject::up;
		if (pro->canMoveInDirection(GraphObject::down) && protesterMaze[bx][by - 1] < protesterMaze[bx][by] && protesterMaze[bx][by - 1] > 0)
			return GraphObject::down;
	}
	return GraphObject::none;

}