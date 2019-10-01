#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "GameConstants.h"
#include <iostream>
using namespace std;

class StudentWorld;

class Actor : public GraphObject {
public:
	Actor(int imageID, double x, double y, StudentWorld* sw, Direction dir = 0, int depth = 0);

	virtual void doSomething() = 0;
		
	bool moveActor(Direction d, int amt);

	Direction followActor(const Actor* a) const;

	// can't make pure virtual because not all derived classes have a damage func
	virtual void damage() {}

	void kill();

	bool isAlive() const;

	void setCollisionStatus(bool cs);

	bool canCollide() const;

	void setInfectabilityStatus(bool is);

	bool canBeInfected() const;

	void setMoveStatus(bool ms);

	bool canMove() const;

	void setFlameResistStatus(bool frs);

	bool resistsFlames() const;

	void setDamageStatus(bool ds);

	bool takesDamage() const;
	
	StudentWorld* getWorld() const;
private:
	bool m_aliveStatus;
	bool m_collisionStatus;
	bool m_infectabilityStatus;
	bool m_moveStatus;
	bool m_flameResistStatus;
	bool m_damageStatus;
	StudentWorld* m_world;
};

class Person : public Actor {
public:
	Person(int imageID, double level_x, double level_y, StudentWorld* sw);

	virtual void doSomething();

	virtual void actionAfterDeath() = 0;

	void incrementInfection();

	int getInfection() const;

	void infect();

	void cure();

private:
	bool m_infectionStatus;
	int m_infectionCounter;
};

class Penelope : public Person {
public:
	Penelope(double level_x, double level_y, StudentWorld* sw);
	
	virtual void doSomething();

	virtual void damage();

	virtual void actionAfterDeath();

	void useVaccine();

	void useFlamethrower();

	void useLandmine();

	int getLandmines() const;

	int getFlames() const;

	void addVaccine();

	void addGasCan();

	void addLandmine();

	int getVaccines() const;
private:
	int m_numLandmines;
	int m_numFlames;
	int m_numVaccines;
};

class Zombie;	// necessary for canAvoidZombie func parameter
class Citizen : public Person {
public:
	Citizen(double level_x, double level_y, StudentWorld* sw);

	virtual void doSomething();

	virtual void damage();

	virtual void actionAfterDeath();

	bool canAvoidZombie(double dist_z_squared, const Zombie* z, Direction& d) const;
};

class Zombie : public Actor {
public:
	Zombie(double x, double y, StudentWorld* sw);

	virtual void doSomething() = 0;

	void newPlanDist();

	void newRandDirection();

	void getVomitCoords(double x, double y, double& vomit_x, double& vomit_y) const;

	bool determineVomit() const;

	void setPlanDirection(Direction d);

	void moveZombie();

	int planDist() const;

private:
	int m_planDistance;
	Direction m_planDirection;
};

class DumbZombie : public Zombie {
public:
	DumbZombie(double x, double y, StudentWorld* sw);

	virtual void doSomething();

	virtual void damage();
};

class SmartZombie : public Zombie {
public:
	SmartZombie(double x, double y, StudentWorld* sw);

	virtual void doSomething();

	virtual void damage();
};

class Wall : public Actor {
public:
	Wall(double level_x, double level_y, StudentWorld* sw);

	virtual void doSomething() {}
};

class Exit : public Actor {
public:
	Exit(double level_x, double level_y, StudentWorld* sw);
	virtual void doSomething();
};

class Goodie : public Actor {
public:
	Goodie(int imageID, double x, double y, StudentWorld* sw);

	virtual void doSomething() = 0;

	virtual void damage();

	bool collected();
};

class Vaccine : public Goodie {
public:
	Vaccine(double x, double y, StudentWorld* sw);

	virtual void doSomething();
};

class GasCan : public Goodie {
public:
	GasCan(double x, double y, StudentWorld* sw);

	virtual void doSomething();
};

class LandmineG : public Goodie {
public:
	LandmineG(double x, double y, StudentWorld* sw);

	virtual void doSomething();
};

class Landmine : public Actor {
public:
	Landmine(double x, double y, StudentWorld* sw);

	virtual void doSomething();

	virtual void damage();
private:
	int m_safetyTicks;
};

class Pit : public Actor {
public:
	Pit(double x, double y, StudentWorld* sw);

	virtual void doSomething();
};

class Projectile : public Actor {
public:
	Projectile(int imageID, int x, int y, StudentWorld* sw, Direction dir);

	virtual void doSomething() = 0;

	bool destroyed();
private:
	bool m_justCreated;
};

class Vomit : public Projectile {
public:
	Vomit(double x, double y, StudentWorld* sw, Direction dir);

	virtual void doSomething();
};

class Flame : public Projectile {
public:
	Flame(double x, double y, StudentWorld* sw, Direction dir);

	virtual void doSomething();
};

#endif // ACTOR_H_
