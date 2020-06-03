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
#include "Craig.h"
#include "Window.h"

using namespace std;
extern fstream filestr;
Initializer<Craig> __Craig_initializer;

String Craig::species_name(void) const
{
	return "Craig";
}

Action Craig::encounter(const ObjInfo& info)
{
	filestr << "at time " << Event::now() << " encounter called for craig for specie:" << info.species << " with" << this << " having health " << health() << endl;
	if (info.species == species_name()) {
		/* don't be cannibalistic */
		set_course(info.bearing + M_PI);
		return LIFEFORM_IGNORE;
	}
	else {
		Delete(hunt_event);
		hunt_event = new Event(0.0, make_procedure(*this, &Craig::hunt));
		filestr << " the value of the event is " << hunt_event;
		return LIFEFORM_EAT;
	}
}

void Craig::initialize(void)
{
	LifeForm::add_creator(Craig::create, "Craig");
}

/*
* REMEMBER: do not call set_course, set_speed, perceive, or reproduce
* from inside the constructor!!!!
* you must wait until the object is actually alive
*/
Craig::Craig()
{
	filestr << "at time " << Event::now() << " encounter called for craig for specie:" << this << " having health " << health() << endl;
	hunt_event = Nil<Event>();
	(void) new Event(0.0, make_procedure(*this, &Craig::live));
}


Craig::~Craig()
{
	//cout<<"Craig's are still surviving and about to die for "<<this<<endl;
}

void Craig::spawn(void)
{
	filestr << "at time " << Event::now() << " spawn called for craig for specie:" << " with" << this << " having health " << health() << endl;
	Craig* child = new Craig;
	reproduce(child);
}


Color Craig::my_color(void) const
{
	return RED;
}

LifeForm* Craig::create(void)
{
	Craig* res = Nil<Craig>();
	res = new Craig;
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
void Craig::live(void)
{
	filestr << "at time " << Event::now() << " live called for craig for specie:" << " with" << this << " having health " << health() << endl;
	set_course(drand48() * 2.0 * M_PI);
	set_speed(2 + 5.0 * drand48());
	hunt_event = new Event(10.0, make_procedure(*this, &Craig::hunt));
	filestr << " the value of the event is " << hunt_event;
}

/*TODO: BUG: check if _die is called for Craig and die callled at a
very earlier stage then why there is a misplacement of
region resize.*/
/*TODO: BUG_CONFIRM: check if craigs are moving away from the prey by setting the
course exactly equal to the course mentioned*/
void Craig::hunt(void)
{
	const String fav_food = "Algae";
	filestr << "at time " << Event::now() << " hunt called for craig for specie:" << " with" << this << " having health " << health() << endl;
	hunt_event = Nil<Event>();
	ObjList prey = perceive(20.0);

	double best_d = HUGE;
	for (ObjList::iterator i = prey.begin(); i != prey.end(); ++i) {
		//if ((*i).species == fav_food) {
		if (!((*i).species == "Craig")) {
			if (best_d > (*i).distance) {
				//filestr<<"course being set after species_name "<<(*i).species<<" at distance "<<(*i).distance<<" for craig "<<this<<endl;
				set_course((*i).bearing);
				best_d = (*i).distance;
			}
		}
	}
	hunt_event = new Event(10.0, make_procedure(*this, &Craig::hunt));
	if (health() >= 4) spawn();
}
