#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include "CraigUtils.h"
#include "Params.h"
#include "Event.h"
#include "ObjInfo.h"
#include "Set.h"
#include "Bait.h"
#include "Window.h"


static int death_count_bait = 1;
static int death_count_shybait = 1;
extern fstream filestr;
using namespace std;

static Initializer<Bait> __Bait_initializer;


string Bait::species_name(void) const
{
	return "Algae";
}

string Bait::player_name(void) const
{
	return "Bait";
}

Action Bait::encounter(const ObjInfo&)
{
	return LIFEFORM_EAT;
}

void Bait::initialize(void)
{
	LifeForm::add_creator(Bait::create, "Bait");
}

/*
* REMEMBER: do not call set_course, set_speed, perceive, or reproduce
* from inside the constructor!!!!
* you must wait until the object is actually alive
*/
Bait::Bait(void)
{
	(void) new Event(20, make_procedure(*this, &Bait::birth));
}

Bait::~Bait(void)
{
	filestr << "Destruction of Baits:" << this << " at death_count " << death_count_bait << endl;
	death_count_bait++;
}

void Bait::wake_up()
{
	if (health() > 20)
		fission();

	/* try to move near other Algae */
	ObjList neighbors = perceive(100.0);
	choose_course(neighbors);
	(void) new Event(5, make_procedure((*this), &Bait::hibernate));
}

void Bait::hibernate()
{
	set_speed(0.0);
	(void) new Event(50, make_procedure((*this), &Bait::wake_up));
	/* go to sleep now */
}

void Bait::fission(void)
{
	Bait* child;

	if (drand48() < 0.2)
		child = new ShyBait;
	else
		child = new Bait;
	reproduce(child);
}

/*void ShyBait::fission(void)
{
Bait* child;
if (drand48() < 0.2)
child = new Bait;
else
child = new ShyBait;
reproduce(child);
}*/

/*
* this event only occurs when a Bait first becomes alive
* when first born, Baits try to move a long way away from their
* parents
*/
void Bait::birth(void)
{
	set_course(2.0 * M_PI * drand48());
	set_speed(max_speed);
	(void) new Event(5, make_procedure((*this), &Bait::hibernate));
}

Color Bait::my_color(void) const
{
	return YELLOW;
}

LifeForm* Bait::create(void)
{
	if (drand48() < 0.5)
		return new Bait;
	else
		return new ShyBait;

}


void Bait::choose_course(ObjList& neighbors)
{
	static int count = 1;
	filestr << "choose course called " << this << " at count " << count << endl;
	count++;
	double best_course = 2.0 * M_PI * drand48();
	double closest = HUGE;
	for (ObjList::iterator i = neighbors.begin(); i != neighbors.end(); ++i) {
		if ((*i).species == static_cast<string>("Algae")
			&& (*i).distance < closest) {
			closest = (*i).distance;
			best_course = (*i).bearing;
		}
	}

	set_course(best_course);
	set_speed(max_speed);             // fast as we can for 5 time units
}

ShyBait::ShyBait() :Bait()
{
}
ShyBait::~ShyBait()
{
	filestr << "Destruction of ShyBaits " << this << " at death count " << death_count_shybait << endl;
	death_count_shybait++;
}
/*void ShyBait::choose_course(ObjList& neighbors)
{
static int count = 1;
filestr<<"choose course called for ShyBait "<<this<<" at count "<<count<<endl;
count++;
double best_course = 2.0 * M_PI * drand48();
double closest = HUGE;
for (ObjList::iterator i = neighbors.begin(); i != neighbors.end(); ++i) {
if ((*i).species == static_cast<string>("Algae")
&& (*i).distance < closest) {
closest = (*i).distance;
best_course = (*i).bearing + M_PI;
}
}

set_course(best_course);
set_speed(max_speed);             // fast as we can for 5 time units
}*/
