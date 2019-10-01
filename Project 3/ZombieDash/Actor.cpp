#include "Actor.h"
#include "StudentWorld.h"
#include <cstdlib>

// Actor constructor
Actor::Actor(int imageID, double x, double y, StudentWorld* sw, Direction dir, int depth)
	: GraphObject(imageID, x, y, dir, depth),
	m_aliveStatus(true), m_world(sw), m_collisionStatus(false), m_infectabilityStatus(false),
	m_moveStatus(false), m_flameResistStatus(false), m_damageStatus(false) {}

//========================================
//	Actor::moveActor
//
//	move actor in specified direction by specified
//	amt and return true if unblocked
//========================================
bool Actor::moveActor(Direction d, int amt) {
	if (!m_moveStatus)
		return false;

	setDirection(d);

	int dest_x = getX(), dest_y = getY();
	switch (d) {
	case right:
		dest_x += amt;
		break;
	case up:
		dest_y += amt;
		break;
	case left:
		dest_x -= amt;
		break;
	case down:
		dest_y -= amt;
		break;
	}

	if (!getWorld()->positionBlocked(dest_x, dest_y, this)) {
		moveTo(dest_x, dest_y);
		return true;
	}
	else
		return false;
}

//========================================
//	Actor::followActor
//
//	determine direction necessary to follow Actor
//========================================
Direction Actor::followActor(const Actor* a) const {
	Direction d1 = right, d2 = right;

	// left or right
	if (getX() < a->getX())
		d1 = right;
	else if (getX() > a->getX())
		d1 = left;

	// same row
	if (getY() == a->getY())
		return d1;

	// up or down
	if (getY() < a->getY())
		d2 = up;
	else if (getY() > a->getY())
		d2 = down;

	// same column
	if (getX() == a->getX())
		return d2;

	// randomly choose d1 or d2
	double r = (double)rand() / RAND_MAX;
	return r < 0.5 ? d1 : d2;
}

void Actor::kill() {
	m_aliveStatus = false;
}

bool Actor::isAlive() const {
	return m_aliveStatus;
}

void Actor::setCollisionStatus(bool cs) {
	m_collisionStatus = cs;
}

bool Actor::canCollide() const {
	return m_collisionStatus;
}

void Actor::setInfectabilityStatus(bool is) {
	m_infectabilityStatus = is;
}

bool Actor::canBeInfected() const {
	return m_infectabilityStatus;
}

void Actor::setMoveStatus(bool ms) {
	m_moveStatus = ms;
}

bool Actor::canMove() const {
	return m_moveStatus;
}

void Actor::setFlameResistStatus(bool frs) {
	m_flameResistStatus = frs;
}

bool Actor::resistsFlames() const {
	return m_flameResistStatus;
}

void Actor::setDamageStatus(bool ds) {
	m_damageStatus = ds;
}

bool Actor::takesDamage() const {
	return m_damageStatus;
}

StudentWorld* Actor::getWorld() const {
	return m_world;
}

Person::Person(int imageID, double level_x, double level_y, StudentWorld* sw)
	: Actor(imageID, level_x*SPRITE_WIDTH, level_y*SPRITE_HEIGHT, sw), m_infectionStatus(false), m_infectionCounter(0) {
	setInfectabilityStatus(true);
	setMoveStatus(true);
	setDamageStatus(true);
}

//========================================
//	Person::doSomething
//
//	increment infection counter if infected
//========================================
void Person::doSomething() {
	if (m_infectionStatus)
		incrementInfection();
	if (!isAlive())
		actionAfterDeath();
}

//========================================
//	Person::incrementInfection
//
//	increment infection counter and possibly die
//========================================
void Person::incrementInfection() {
	m_infectionCounter++;
	if (m_infectionCounter >= 500)
		kill();
}

// m_infectionCounter accessor
int Person::getInfection() const {
	return m_infectionCounter;
}

void Person::infect() {
	m_infectionStatus = true;
}

void Person::cure() {
	m_infectionStatus = false;
	m_infectionCounter = 0;
}

Penelope::Penelope(double level_x, double level_y, StudentWorld* sw)
	: Person(IID_PLAYER, level_x, level_y, sw),
	m_numLandmines(0), m_numFlames(0), m_numVaccines(0) {}

//========================================
//	Penelope::doSomething
//
//	read key input and decide what Penelope should do
//========================================
void Penelope::doSomething() {
	// handle infection counter
	Person::doSomething();
	if (!isAlive())
		return;

	int keyValue;

	// categorize any key press value
	if (getWorld()->getKey(keyValue)) {
		switch (keyValue) {
		case KEY_PRESS_RIGHT:
			moveActor(right, 4);
			break;
		case KEY_PRESS_UP:
			moveActor(up, 4);
			break;
		case KEY_PRESS_LEFT:
			moveActor(left, 4);
			break;
		case KEY_PRESS_DOWN:
			moveActor(down, 4);
			break;
		case KEY_PRESS_ENTER:
			useVaccine();
			break;
		case KEY_PRESS_SPACE:
			useFlamethrower();
			break;
		case KEY_PRESS_TAB:
			useLandmine();
			break;
		default:
			cerr << "ERROR: invalid key press." << endl;
		}
	}
}

//========================================
//	Penelope::damage
//
//	Penelope dies
//========================================
void Penelope::damage() {
	getWorld()->playSound(SOUND_PLAYER_DIE);
	kill();
}

//========================================
//	Penelope::actionAfterDeath
//
//	play death sound. StudentWorld handles the rest.
//========================================
void Penelope::actionAfterDeath() {
	getWorld()->playSound(SOUND_PLAYER_DIE);
}

//========================================
//	Penelope::useVaccine
//
//	use a vaccine to cure Penelope
//========================================
void Penelope::useVaccine() {
	if (m_numVaccines > 0) {
		cure();
		m_numVaccines--;
	}
}

//========================================
//	Penelope::useFlamethrower
//
//	use a flamethrower to kill zombies or
//	citizens in front of Penelope
//========================================
void Penelope::useFlamethrower() {
	// cannot use flamethrower without flames
	if (m_numFlames <= 0)
		return;

	// flamethrower has been used
	getWorld()->playSound(SOUND_PLAYER_FIRE);
	m_numFlames--;

	// generate up to 3 flames if unblocked
	for (int i = 1; i <= 3; i++) {
		switch (getDirection()) {
		case right:
			if (!getWorld()->canAddFlame(getX() + i * SPRITE_WIDTH, getY(), getDirection()))
				return;
			break;
		case up:
			if (!getWorld()->canAddFlame(getX(), getY() + i * SPRITE_HEIGHT, getDirection()))
				return;
			break;
		case left:
			if (!getWorld()->canAddFlame(getX() - i * SPRITE_WIDTH, getY(), getDirection()))
				return;
			break;
		case down:
			if (!getWorld()->canAddFlame(getX(), getY() - i * SPRITE_HEIGHT, getDirection()))
				return;
			break;
		default:
			cerr << "ERROR: invalid direction held by Penelope: " << getDirection() << endl;
			exit(1);
		}
	}
}

//========================================
//	Penelope::useLandmine
//
//	add a landmine at current coordinates
//========================================
void Penelope::useLandmine() {
	if (m_numLandmines > 0) {
		getWorld()->addLandmine(getX(), getY());
		m_numLandmines--;
	}
}

int Penelope::getLandmines() const {
	return m_numLandmines;
}

int Penelope::getFlames() const {
	return m_numFlames;
}

void Penelope::addVaccine() {
	m_numVaccines++;
}

void Penelope::addGasCan() {
	m_numFlames += 5;
}

void Penelope::addLandmine() {
	m_numLandmines += 2;
}

int Penelope::getVaccines() const {
	return m_numVaccines;
}

Citizen::Citizen(double level_x, double level_y, StudentWorld* sw)
	: Person(IID_CITIZEN, level_x, level_y, sw) {}

//========================================
//	Citizen::doSomething
//
//	decide what citizen should do
//========================================
void Citizen::doSomething() {
	// handle infection counter
	Person::doSomething();
	if (!isAlive())
		return;

	// play citizen infected sound if just infected
	if (getInfection() == 1)
		getWorld()->playSound(SOUND_CITIZEN_INFECTED);

	// paralyzed if even tick
	if (getWorld()->evenTick())
		return;

	// calculate dist_p and dist_z
	double dist_p_squared = getWorld()->distanceSquared(getX(), getY(), 
		getWorld()->getPenelope()->getX(), getWorld()->getPenelope()->getY());

	Zombie* z;
	Direction d;
	double dist_z_squared = getWorld()->distanceSquaredToZombie(this, z);

	// move toward Penelope or away from a zombie
	if (dist_p_squared < dist_z_squared && dist_p_squared <= 6400)
		moveActor(followActor(getWorld()->getPenelope()), 2);
	else if (dist_z_squared <= 6400 && canAvoidZombie(dist_z_squared, z, d))
		moveActor(d, 2);
}

//========================================
//	Citizen::damage
//
//	citizen dies
//========================================
void Citizen::damage() {
	getWorld()->playSound(SOUND_CITIZEN_DIE);
	getWorld()->increaseScore(-1000);
	kill();
}

//========================================
//	Citizen::actionAfterDeath
//
//	incomplete
//========================================
void Citizen::actionAfterDeath() {
	getWorld()->playSound(SOUND_ZOMBIE_BORN);
	getWorld()->increaseScore(-1000);
	getWorld()->addZombie(getX(), getY());
}

//========================================
//	Citizen::canAvoidZombie
//
//	determines if citizen can move 2 pixels to
//	avoid the zombie. d is set to direction with
//	max distance.
//========================================
bool Citizen::canAvoidZombie(double dist_z_squared, const Zombie* z, Direction& d) const {
	double maxDist = dist_z_squared;
	double tempDist;

	// right
	if (!getWorld()->positionBlocked(getX() + 2, getY(), this) && (tempDist =
		getWorld()->distanceSquared(getX()+2, getY(), z->getX(), z->getY())) > maxDist) {
		maxDist = tempDist;
		d = right;
	}

	// up
	if (!getWorld()->positionBlocked(getX(), getY() + 2, this) && (tempDist =
		getWorld()->distanceSquared(getX(), getY()+2, z->getX(), z->getY())) > maxDist) {
		maxDist = tempDist;
		d = up;
	}

	// left
	if (!getWorld()->positionBlocked(getX() - 2, getY(), this) && (tempDist =
		getWorld()->distanceSquared(getX()-2, getY(), z->getX(), z->getY())) > maxDist) {
		maxDist = tempDist;
		d = left;
	}

	// down
	if (!getWorld()->positionBlocked(getX(), getY() - 2, this) && (tempDist =
		getWorld()->distanceSquared(getX(), getY()-2, z->getX(), z->getY())) > maxDist) {
		maxDist = tempDist;
		d = down;
	}

	return maxDist != dist_z_squared;
}

Zombie::Zombie(double x, double y, StudentWorld* sw)
	: Actor(IID_ZOMBIE, x, y, sw), m_planDistance(0) {
	setMoveStatus(true);
	setDamageStatus(true);
}

//========================================
//	Zombie::newMovePlan
//
//	randomly determine a new planned distance
//	between 3 and 10 inclusive
//========================================
void Zombie::newPlanDist() {
	m_planDistance = (double)rand() / RAND_MAX * 8 + 3;
}

//========================================
//	Zombie::newRandDirection
//
//	randomly determine new direction
//========================================
void Zombie::newRandDirection() {
	switch ((int)((double)rand() / RAND_MAX * 4)) {
	case 0:
		m_planDirection = right;
		break;
	case 1:
		m_planDirection = up;
		break;
	case 2:
		m_planDirection = left;
		break;
	case 3:
		m_planDirection = down;
		break;
	default:
		cerr << "ERROR: invalid random int in Zombie::newMovePlan()" << endl;
		exit(1);
	}
}

//========================================
//	Zombie::getVomitCoords
//
//	determine the coordinates for vomit to go
//========================================
void Zombie::getVomitCoords(double x, double y, double& vomit_x, double& vomit_y) const {
	vomit_x = x;
	vomit_y = y;
	switch (getDirection()) {
	case right:
		vomit_x += SPRITE_WIDTH;
		break;
	case up:
		vomit_y += SPRITE_HEIGHT;
		break;
	case left:
		vomit_x -= SPRITE_WIDTH;
		break;
	case down:
		vomit_y -= SPRITE_HEIGHT;
		break;
	default:
		cerr << "ERROR: invalid direction " << getDirection();
		exit(1);
	}
}

//========================================
//	Zombie::determineVomit
//
//	determine whether or not a person is close
//	enought to create vomit
//========================================
bool Zombie::determineVomit() const {
	double vomit_x, vomit_y;
	getVomitCoords(getX(), getY(), vomit_x, vomit_y);

	if (getWorld()->canVomitOnPerson(vomit_x, vomit_y) && (double)rand() / RAND_MAX * 3 < 1) {
		getWorld()->addVomit(vomit_x, vomit_y, m_planDirection);
		return true;
	}

	return false;
}

void Zombie::setPlanDirection(Direction d) {
	m_planDirection = d;
}

//========================================
//	Zombie::moveZombie
//
//	move the zombie and handle distance and direction
//========================================
void Zombie::moveZombie() {
	moveActor(m_planDirection, 1) ? m_planDistance-- : m_planDistance = 0;
}

int Zombie::planDist() const {
	return m_planDistance;
}

DumbZombie::DumbZombie(double x, double y, StudentWorld* sw)
	: Zombie(x, y, sw) {}

//========================================
//	DumbZombie::doSomething
//
//	determine vomit and random movement
//========================================
void DumbZombie::doSomething() {
	// paralyzed if even tick
	if (getWorld()->evenTick())
		return;

	// check vomit
	if (determineVomit())
		return;

	// choose new movement plan
	if (planDist() == 0) {
		newPlanDist();
		newRandDirection();
	}

	// move if possible
	moveZombie();
}

//========================================
//	DumbZombie::damage
//
//	zombie dies and has 10% chance to drop a vaccine
//========================================
void DumbZombie::damage() {
	getWorld()->playSound(SOUND_ZOMBIE_DIE);
	getWorld()->increaseScore(1000);

	if ((double)rand() / RAND_MAX < 0.1)
		getWorld()->dropVaccine(getX(), getY());

	kill();
}

SmartZombie::SmartZombie(double x, double y, StudentWorld* sw)
	: Zombie(x, y, sw) {}

//========================================
//	SmartZombie::doSomething
//
//	determine vomit and random/calculated movement
//========================================
void SmartZombie::doSomething() {
	//paralyzed if even tick
	if (getWorld()->evenTick())
		return;

	// check vomit
	if (determineVomit())
		return;

	//choose new movement plan
	if (planDist() == 0) {
		newPlanDist();
		Person* p;
		if (getWorld()->distanceSquaredToPerson(this, p) <= 6400)
			setPlanDirection(followActor(p));
		else
			newRandDirection();
	}

	// move if possible
	moveZombie();
}

//========================================
//	SmartZombie::damage
//
//	zombie dies
//========================================
void SmartZombie::damage() {
	getWorld()->playSound(SOUND_ZOMBIE_DIE);
	getWorld()->increaseScore(2000);
	kill();
}

Wall::Wall(double level_x, double level_y, StudentWorld* sw)
	: Actor(IID_WALL, level_x*SPRITE_WIDTH, level_y*SPRITE_HEIGHT, sw) {
	setFlameResistStatus(true);
}

Exit::Exit(double level_x, double level_y, StudentWorld* sw)
	: Actor(IID_EXIT, level_x*SPRITE_WIDTH, level_y*SPRITE_HEIGHT, sw, 0, 1) {
	setCollisionStatus(true);
	setFlameResistStatus(true);
}

//========================================
//	Exit::doSomething
//
//	checks if a citizen or Penelope is on this
//========================================
void Exit::doSomething() {
	// citizen on exit
	getWorld()->citizenOnExit(this);

	// Penelope on exit and no citizens alive
	if (getWorld()->overlaps(getX(), getY(), getWorld()->getPenelope()->getX(), getWorld()->getPenelope()->getY())
		&& !getWorld()->citizensAlive()) {
		getWorld()->completeLevel();
	}
}

Goodie::Goodie(int imageID, double x, double y, StudentWorld* sw)
	: Actor(imageID, x, y, sw, 0, 1) {
	setCollisionStatus(true);
	setDamageStatus(true);
}

//========================================
//	Goodie::damage
//
//	goodie dies
//========================================
void Goodie::damage() {
	kill();
}

//========================================
//	Goodie::collected
//
//	if overlapping with Penelope, complete
//	goodie collection actions and return true.
//========================================
bool Goodie::collected() {
	if (getWorld()->overlaps(getX(), getY(),
		getWorld()->getPenelope()->getX(), getWorld()->getPenelope()->getY())) {
		getWorld()->increaseScore(50);
		kill();
		getWorld()->playSound(SOUND_GOT_GOODIE);
		return true;
	}
	return false;
}

Vaccine::Vaccine(double x, double y, StudentWorld* sw)
	: Goodie(IID_VACCINE_GOODIE, x, y, sw) {}

//========================================
//	Vaccine::doSomething
//
//	give Penelope a vaccine if collected
//========================================
void Vaccine::doSomething() {
	if (collected())
		getWorld()->getPenelope()->addVaccine();
}

GasCan::GasCan(double x, double y, StudentWorld* sw)
	: Goodie(IID_GAS_CAN_GOODIE, x, y, sw) {}

//========================================
//	GasCan::doSomething
//
//	give Penelope 5 flames if collected
//========================================
void GasCan::doSomething() {
	if (collected())
		getWorld()->getPenelope()->addGasCan();
}

LandmineG::LandmineG(double x, double y, StudentWorld* sw)
	: Goodie(IID_LANDMINE_GOODIE, x, y, sw) {}

//========================================
//	LandmineG::doSomething
//
//	give Penelope 2 landmines if collected
//========================================
void LandmineG::doSomething() {
	if (collected())
		getWorld()->getPenelope()->addLandmine();
}

Landmine::Landmine(double x, double y, StudentWorld* sw)
	: Actor(IID_LANDMINE, x, y, sw, 0, 1), m_safetyTicks(30) {
	setCollisionStatus(true);
	setDamageStatus(true);
}

//========================================
//	Landmine::doSomething
//
//	explodes once there are no more safety ticks
//========================================
void Landmine::doSomething() {
	if (m_safetyTicks > 0) {
		m_safetyTicks--;
		return;
	}

	getWorld()->explodeLandmine(this);
}

//========================================
//	Landmine::damage
//
//	explodes the landmine, adds 9 flames
//	and a pit
//========================================
void Landmine::damage() {
	kill();
	getWorld()->playSound(SOUND_LANDMINE_EXPLODE);
	
	double x = getX() - SPRITE_WIDTH, y = getY() - SPRITE_HEIGHT;
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			getWorld()->addFlame(x + j * SPRITE_WIDTH, y + i * SPRITE_HEIGHT, up);

	getWorld()->addPit(getX(), getY());
}

Pit::Pit(double x, double y, StudentWorld* sw)
	: Actor(IID_PIT, x, y, sw) {
	setCollisionStatus(true);
}

//========================================
//	Pit::doSomething
//
//	kill all overlapping actors using damage
//	function
//========================================
void Pit::doSomething() {
	getWorld()->damageAll(getX(), getY());
}

Projectile::Projectile(int imageID, int x, int y, StudentWorld* sw, Direction dir)
	: Actor(imageID, x, y, sw, dir), m_justCreated(true) {
	setCollisionStatus(true);
}

//========================================
//	Projectile::destroyed
//
//	die 2 ticks after creation and return true
//	when this occurrs
//========================================
bool Projectile::destroyed() {
	if (!m_justCreated) {
		kill();
		return true;
	}
	m_justCreated = false;
	return false;
}

Vomit::Vomit(double x, double y, StudentWorld* sw, Direction dir)
	: Projectile(IID_VOMIT, x, y, sw, dir) {}

//========================================
//	Vomit::doSomething
//
//	infect the overlapping person
//========================================
void Vomit::doSomething() {
	if (destroyed())
		return;
	getWorld()->infectPerson(getX(), getY());
}

Flame::Flame(double x, double y, StudentWorld* sw, Direction dir)
	: Projectile(IID_FLAME, x, y, sw, dir) {}

//========================================
//	Flame::doSomething
//
//	damage all overlapping damageable actors
//========================================
void Flame::doSomething() {
	if (destroyed())
		return;

	getWorld()->damageAll(getX(), getY());
}