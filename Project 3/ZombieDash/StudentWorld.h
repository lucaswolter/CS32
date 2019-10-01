#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Actor.h"
#include "Level.h"
#include <string>
#include <vector>
using namespace std;

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
public:
    StudentWorld(string assetPath);
    virtual int init();
    virtual int move();
    virtual void cleanUp();
	virtual ~StudentWorld();

	bool positionBlocked(double x, double y, const Actor* addr) const;
	double distanceSquared(double x1, double y1, double x2, double y2) const;
	bool overlaps(double x1, double y1, double x2, double y2) const;
	bool intersect(double x1, double y1, double x2, double y2) const;

	void citizenOnExit(const Exit* e);

	bool canVomitOnPerson(double x, double y) const;
	void infectPerson(double x, double y);
	void addVomit(double x, double y, Direction d);
	void dropVaccine(double x, double y);

	void explodeLandmine(Landmine* l);
	void damageAll(double x, double y);
	bool canAddFlame(double x, double y, Direction d);
	void addFlame(double x, double y, Direction d);

	void addZombie(double x, double y);
	void addVaccine(double x, double y);
	void addLandmine(double x, double y);
	void addPit(double x, double y);

	double distanceSquaredToZombie(const Citizen* c, Zombie*& z) const;
	double distanceSquaredToPerson(const Actor* a, Person*& p) const;

	Penelope* getPenelope() const;
	bool evenTick() const;
	bool citizensAlive() const;
	void completeLevel();
private:
	vector<Actor*> m_actorList;
	bool m_levelComplete;
	bool m_evenTick;

	string getLevelFileName() const;
	string getStatText() const;
	void createActor(Level::MazeEntry actorType, int x, int y);
};

#endif // STUDENTWORLD_H_
