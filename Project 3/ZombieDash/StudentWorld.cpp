#include "StudentWorld.h"
#include "GameConstants.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

StudentWorld::StudentWorld(string assetPath)
	: GameWorld(assetPath), m_levelComplete(false)
{
}

//========================================
// init
//
// load current level and create all actors
//	returns GWSTATUS_CONTINUE_GAME
//========================================
int StudentWorld::init() {
	// load current level
	string levelFileName = getLevelFileName();
	Level currentLevel(assetPath());
	Level::LoadResult loadedLevel = currentLevel.loadLevel(levelFileName);

	// check if file path is correct
	if (getLevel() > 99)
		return GWSTATUS_PLAYER_WON;
	if (loadedLevel == Level::load_fail_bad_format) {
		cerr << "ERROR: File " << levelFileName << " was formatted incorrectly." << endl;
		return GWSTATUS_LEVEL_ERROR;
	}
	else if (loadedLevel == Level::load_fail_file_not_found) {
		cerr << "ERROR: File " << levelFileName << " was not found." << endl;
		return GWSTATUS_PLAYER_WON;
	}
	else if (loadedLevel == Level::load_success) {
		cerr << "Successfully loaded File " << levelFileName << "." << endl;

		// create all actors
		for(int x = 0; x < LEVEL_WIDTH; x++)	// change LEVEL_WIDTH if necessary
			for(int y=0; y < LEVEL_HEIGHT; y++)	// change LEVEL_HEIGHT if necessary
				createActor(currentLevel.getContentsOf(x, y), x, y);
	}
	else {
		cerr << "ERROR: unknown in StudentWorld::init()" << endl;
		exit(2);
	}

    return GWSTATUS_CONTINUE_GAME;
}

//========================================
//	move
//
//	Represents one game tick.
//	Each alive actor does something, all dead
//	actors are removed, and the display text is
//	updated.
//	Returns GameWorld status: player died, finished
//	level, or continue game.
//========================================
int StudentWorld::move() {
	int origSize = m_actorList.size();
	for (int i=0; i<origSize; i++) {
		if (m_actorList[i]->isAlive()) {
			// each actor does something
			m_actorList[i]->doSomething();

			// check if Penelope died
			if (!m_actorList[0]->isAlive()) {
				decLives();
				return GWSTATUS_PLAYER_DIED;
			}

			// check if level is complete
			if (m_levelComplete) {
				m_levelComplete = false;
				playSound(SOUND_LEVEL_FINISHED);
				return GWSTATUS_FINISHED_LEVEL;
			}
		}
	}

	// remove all dead actors
	for (vector<Actor*>::iterator it = m_actorList.begin();
		it != m_actorList.end();) {
		if ((*it)->isAlive())
			it++;
		else {
			delete *it;
			it = m_actorList.erase(it);
		}
	}

	// update stat text
	setGameStatText(getStatText());

	// change even tick
	m_evenTick ? m_evenTick = false : m_evenTick = true;

	return GWSTATUS_CONTINUE_GAME;
}

//========================================
//	cleanUp
//
//	deallocate every Actor in m_actorList
//========================================
void StudentWorld::cleanUp() {
	for (vector<Actor*>::iterator it = m_actorList.begin();
		it != m_actorList.end(); it = m_actorList.erase(it))
		delete *it;
}

StudentWorld::~StudentWorld() {
	cleanUp();
}

//========================================
//	positionBlocked
//
//	determines if given coordinates are occupied
//	by an actor that cannot collide.
//========================================
bool StudentWorld::positionBlocked(double x, double y, const Actor* addr) const {
	for (int i = 0; i < m_actorList.size(); i++) {
		if (addr == m_actorList[i])	// prevent checking with same Actor
			continue;
		else if (!m_actorList[i]->canCollide() && intersect(x, y, m_actorList[i]->getX(), m_actorList[i]->getY()))
			return true;
	}
	return false;
}

//========================================
//	distanceSquared
//
//	return the euclidian distance between 2 coordinates
//========================================
double StudentWorld::distanceSquared(double x1, double y1, double x2, double y2) const {
	return (x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2);
}

//========================================
//	overlaps
//
//	coordinates overlap if dx2 + dy2 <= 100
//========================================
bool StudentWorld::overlaps(double x1, double y1, double x2, double y2) const {
	return distanceSquared(x1, y1, x2, y2) <= 100;
}

//========================================
//	intersect
//
//	returns true if both sets of coordinates intersect
//========================================
bool StudentWorld::intersect(double x1, double y1, double x2, double y2) const {
	return x1 <= x2 + SPRITE_WIDTH - 1 && x1 + SPRITE_WIDTH - 1 >= x2 &&
		y1 <= y2 + SPRITE_HEIGHT - 1 && y1 + SPRITE_HEIGHT - 1 >= y2;
}

//========================================
//	citizenOnExit
//
//	kills any citizens on a given exit without
//	deducting score
//========================================
void StudentWorld::citizenOnExit(const Exit* e) {
	for (int i = 1; i < m_actorList.size(); i++) {
		if (m_actorList[i]->canBeInfected() && overlaps(e->getX(), e->getY(),
			m_actorList[i]->getX(), m_actorList[i]->getY())) {
			playSound(SOUND_CITIZEN_SAVED);
			increaseScore(500);
			m_actorList[i]->kill();
		}
	}
}

//========================================
//	canVomitOnPerson
//
//	determine if a person is close enough
//	to be vomited on
//========================================
bool StudentWorld::canVomitOnPerson(double x, double y) const {
	for (int i = 0; i < m_actorList.size(); i++) {
		if (m_actorList[i]->canBeInfected() && overlaps(x, y, m_actorList[i]->getX(), m_actorList[i]->getY()))
			return true;
	}
	return false;
}

//========================================
//	infectPerson
//
//	infect any person overlapping the vomit
//========================================
void StudentWorld::infectPerson(double x, double y) {
	for (int i = 0; i < m_actorList.size(); i++) {
		if (m_actorList[i]->canBeInfected() && overlaps(x, y,
			m_actorList[i]->getX(), m_actorList[i]->getY()))
			static_cast<Person*>(m_actorList[i])->infect();
	}
}

//========================================
//	addVomit
//
//	create vomit at the specified coordinates
//	with the specified direction
//========================================
void StudentWorld::addVomit(double x, double y, Direction d) {
	playSound(SOUND_ZOMBIE_VOMIT);
	m_actorList.push_back(new Vomit(x, y, this, d));
}

//========================================
//	dropVaccine
//
//	throw a vaccine in a random direction
//	from where the dumb zombie dies
//========================================
void StudentWorld::dropVaccine(double x, double y) {
	double rDir = (double)rand() / RAND_MAX;
	if (rDir < 0.25)	// right
		x += SPRITE_WIDTH;
	else if (rDir < 0.5)	// up
		y += SPRITE_HEIGHT;
	else if (rDir < 0.75)	// left
		x -= SPRITE_WIDTH;
	else	// down
		y -= SPRITE_HEIGHT;

	for (int i = 0; i < m_actorList.size(); i++) {
		if (overlaps(x, y, m_actorList[i]->getX(), m_actorList[i]->getY()))
			return;
	}

	addVaccine(x, y);
}

//========================================
//	explodeLandmine
//
//	damage the given landmine if it overlaps
//	with a moving actor
//========================================
void StudentWorld::explodeLandmine(Landmine* l) {
	for (int i = 0; i < m_actorList.size(); i++) {
		if (m_actorList[i]->takesDamage() && m_actorList[i]->canMove() &&
			overlaps(l->getX(), l->getY(), m_actorList[i]->getX(), m_actorList[i]->getY())) {
			l->damage();
			break;
		}
	}
}

//========================================
//	damageAll
//
//	damage all damageable actors overlapping
//	the specified coordinates
//========================================
void StudentWorld::damageAll(double x, double y) {
	// to prevent newly created vaccines from being damaged
	int origSize = m_actorList.size();

	for (int i = 0; i < origSize; i++) {
		if (m_actorList[i]->takesDamage() && overlaps(x, y,
			m_actorList[i]->getX(), m_actorList[i]->getY()))
			m_actorList[i]->damage();
	}
}

//========================================
//	canAddFlame
//
//	adds a flame at specified coordinates and
//	direction and returns true if possible
//========================================
bool StudentWorld::canAddFlame(double x, double y, Direction d) {
	for (int i = 0; i < m_actorList.size(); i++) {
		if (m_actorList[i]->resistsFlames() && overlaps(x, y,
			m_actorList[i]->getX(), m_actorList[i]->getY()))
			return false;
	}
	addFlame(x, y, d);
	return true;
}

//========================================
//	addFlame
//
//	create flame at specified coordinates
//	with specified direction
//========================================
void StudentWorld::addFlame(double x, double y, Direction d) {
	m_actorList.push_back(new Flame(x, y, this, d));
}

//========================================
//	addZombie
//
//	create a zombie at the specified coordinates
//========================================
void StudentWorld::addZombie(double x, double y) {
	if ((double)rand() / RAND_MAX < 0.7)
		m_actorList.push_back(new DumbZombie(x, y, this));
	else
		m_actorList.push_back(new SmartZombie(x, y, this));
}

//========================================
//	addVaccine
//
//	create a vaccine at the specified coordinates
//========================================
void StudentWorld::addVaccine(double x, double y) {
	m_actorList.push_back(new Vaccine(x, y, this));
}

//========================================
//	addLandmine
//
//	create a landmine at the specified coordinates
//========================================
void StudentWorld::addLandmine(double x, double y) {
	m_actorList.push_back(new Landmine(x, y, this));
}

//========================================
//	addPit
//
//	create a pit at the specified coordinates
//========================================
void StudentWorld::addPit(double x, double y) {
	m_actorList.push_back(new Pit(x, y, this));
}

//========================================
//	distanceSquaredToZombie
//
//	returns the distance to the nearest zombie
//	under 6401, or returns 6401. z is set to nearest
//	zombie.
//========================================
double StudentWorld::distanceSquaredToZombie(const Citizen* c, Zombie*& z) const {
	// greater than 6400, the minimum to affect citizen movement
	double minDist = 6401;
	double tempDist;
	z = nullptr;

	for (int i = 0; i < m_actorList.size(); i++) {
		if (m_actorList[i]->canMove() && !m_actorList[i]->canBeInfected()	// is a zombie
			&& (tempDist = distanceSquared(c->getX(), c->getY(), m_actorList[i]->getX(), m_actorList[i]->getY())) < minDist) {	// has shortest distance
			z = static_cast<Zombie*>(m_actorList[i]);
			minDist = tempDist;
		}
	}
	return minDist;
}

//========================================
//	distanceSquaredToPerson
//
//	returns the distance to the nearest person
//	under 6401, or returns 6401. p is set to nearest
//	person.
//========================================
double StudentWorld::distanceSquaredToPerson(const Actor* a, Person*& p) const {
	// greater than 6400, the minimum to affect SmartZombie movement
	double minDist = 6401;
	double tempDist;
	p = nullptr;

	for (int i = 0; i < m_actorList.size(); i++) {
		if (m_actorList[i]->canBeInfected()	// is a person
			&& (tempDist = distanceSquared(a->getX(), a->getY(), m_actorList[i]->getX(), m_actorList[i]->getY())) < minDist) {	// has shortest distance
			p = static_cast<Person*>(m_actorList[i]);
			minDist = tempDist;
		}
	}
	return minDist;
}

// Penelope accessor
Penelope* StudentWorld::getPenelope() const {
	return static_cast<Penelope*>(m_actorList[0]);
}

// evenTick accessor
bool StudentWorld::evenTick() const {
	return m_evenTick;
}

// returns true if any citizens are alive
bool StudentWorld::citizensAlive() const {
	for (int i = 1; i < m_actorList.size(); i++) {
		if (m_actorList[i]->canBeInfected())
			return true;
	}
	return false;
}

// levelComplete mutator
void StudentWorld::completeLevel() {
	m_levelComplete = true;
}

//========================================
//	getLevelFileName
//
//	returns the full path of the current level
//	includes necessary: <string>, <iostream>, <sstream>, <iomanip>, "GameWorld.h"
//========================================
string StudentWorld::getLevelFileName() const {
	ostringstream toFileName;
	toFileName.fill('0');
	toFileName << "level" << setw(2) << getLevel() << ".txt";
	return toFileName.str();
}

//========================================
//	getStatText
//
//	returns the full stat text line
//========================================
string StudentWorld::getStatText() const {
	ostringstream toStatText;
	toStatText.fill('0');

	toStatText << "Score: ";
	if (getScore() < 0)
		toStatText << "-" << setw(5) << abs(getScore());
	else
		toStatText << setw(6) << getScore();

	toStatText << "  Level: " << getLevel() << "  Lives: " << getLives() << "  Vaccines: "
		<< getPenelope()->getVaccines() << "  Flames: " << getPenelope()->getFlames()
		<< "  Mines: " << getPenelope()->getLandmines() << "  Infected: " << getPenelope()->getInfection();
	return toStatText.str();
}

//========================================
//	createActor
//
//	dynamically allocates the specified actorType
//	at the specified coordinates and adds it to
//	m_actorList.
//========================================
void StudentWorld::createActor(Level::MazeEntry actorType, int x, int y) {
	switch (actorType) {
	case Level::empty:
		cerr << x << "," << y << " is empty." << endl;
		break;
	case Level::player:
		cerr << x << "," << y << " is Penelope." << endl;
		m_actorList.insert(m_actorList.begin(), new Penelope(x, y, this));
		break;
	case Level::citizen:
		cerr << x << "," << y << " is a citizen." << endl;
		m_actorList.push_back(new Citizen(x, y, this));
		break;
	case Level::dumb_zombie:
		cerr << x << "," << y << " is a dumb zombie." << endl;
		m_actorList.push_back(new DumbZombie(x*SPRITE_WIDTH, y*SPRITE_HEIGHT, this));
		break;
	case Level::smart_zombie:
		cerr << x << "," << y << " is a smart zombie." << endl;
		m_actorList.push_back(new SmartZombie(x*SPRITE_WIDTH, y*SPRITE_HEIGHT, this));
		break;
	case Level::wall:
		cerr << x << "," << y << " is a wall." << endl;
		m_actorList.push_back(new Wall(x, y, this));
		break;
	case Level::pit:
		cerr << x << "," << y << " is a pit." << endl;
		m_actorList.push_back(new Pit(x*SPRITE_WIDTH, y*SPRITE_HEIGHT, this));
		break;
	case Level::exit:
		cerr << x << "," << y << " is an exit." << endl;
		m_actorList.push_back(new Exit(x, y, this));
		break;
	case Level::vaccine_goodie:
		cerr << x << "," << y << " is a vaccine." << endl;
		m_actorList.push_back(new Vaccine(x*SPRITE_WIDTH, y*SPRITE_HEIGHT, this));
		break;
	case Level::gas_can_goodie:
		cerr << x << "," << y << " is a gas can." << endl;
		m_actorList.push_back(new GasCan(x*SPRITE_WIDTH, y*SPRITE_HEIGHT, this));
		break;
	case Level::landmine_goodie:
		cerr << x << "," << y << " is a landmine." << endl;
		m_actorList.push_back(new LandmineG(x*SPRITE_WIDTH, y*SPRITE_HEIGHT, this));
		break;
	default:
		cerr << "ERROR: invalid actor type. " << x << "," << y << " treated as empty." << endl;
	}
}