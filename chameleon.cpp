#include <iostream>
#include <event.h>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include "CraigUtils.h"
#include "Params.h"
#include "Event.h"
#include "String.h"
#include "ObjInfo.h"
#include "Set.h"
#include "chameleon.h"
#include "Window.h"

using namespace std;
extern fstream filestr;
Initializer<Chameleon> __Craig_initializer;

String Chameleon::species_name(void) const
{
	return species;
}

Action Chameleon::encounter(const ObjInfo& info)
{
	this->species = info.species;
	filestr << "species name of chameleon: " << this << " turns out to be " << species << endl;
	if (info.their_speed == get_speed()) {
		/* don't be cannibalistic */
		set_course(info.bearing + M_PI);
		return LIFEFORM_IGNORE;
	}
	else {
		Delete(hunt_event);
		hunt_event = new Event(0.0, make_procedure(*this, &Chameleon::hunt));
		return LIFEFORM_EAT;
	}
}

void Chameleon::initialize(void)
{
	LifeForm::add_creator(Chameleon::create, "Chameleon");
}

/*
* REMEMBER: do not call set_course, set_speed, perceive, or reproduce
* from inside the constructor!!!!
* you must wait until the object is actually alive
*/
Chameleon::Chameleon()
{
	hunt_event = Nil<Event>();
	(void) new Event(0.0, make_procedure(*this, &Chameleon::live));
	species = "Chameleon";
}


Chameleon::~Chameleon()
{
	cout << "Chameleon's are still surviving and about to die for " << this << endl;
}

void Chameleon::spawn(void)
{
	Chameleon* child = new Chameleon;
	reproduce(child);
}


Color Chameleon::my_color(void) const
{
	return MAGENTA;
}

LifeForm* Chameleon::create(void)
{
	Chameleon* res = Nil<Chameleon>();
	res = new Chameleon;
	res->display();
	return res;
}


/*
* this event is only called once, when the object is "born"
*
* I don't want to call "set_course" and "set_speed" from the constructor
* 'cause OBJECTS ARE NOT ALIVE WHEN THEY ARE CONSTRUCTED (remember!)
* so... in the constructor I schedule this event to happen in 0 time
* units
*/
void Chameleon::live(void)
{
	set_course(drand48() * 2.0 * M_PI);
	set_speed(1.7);
	hunt_event = new Event(10.0, make_procedure(*this, &Chameleon::hunt));
}

/*TODO: BUG: check if _die is called for Craig and die callled at a
very earlier stage then why there is a misplacement of
region resize.*/
/*TODO: BUG_CONFIRM: check if craigs are moving away from the prey by setting the
course exactly equal to the course mentioned*/
void Chameleon::hunt(void)
{
	const String fav_food = "Algae";
	filestr << "hunt called for Chameleons for " << this << " at time : "
		<< Event::now() << endl;
	hunt_event = Nil<Event>();
	ObjList prey = perceive(20.0);

	double best_d = HUGE;
	for (ObjList::iterator i = prey.begin(); i != prey.end(); ++i) {
		if ((*i).species == fav_food) {
			if (best_d > (*i).distance) {
				set_course((*i).bearing);
				best_d = (*i).distance;
			}
		}
	}
	hunt_event = new Event(10.0, make_procedure(*this, &Chameleon::hunt));
	if (health() >= 4) spawn();
}
