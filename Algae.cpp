#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <fstream>
#include "Params.h"
#include "Algae.h"
#include "Event.h"
#include "String.h"
#include "Window.h"
#include "tokens.h"

using namespace std;

Initializer<Algae> __Algae_initializer;
extern fstream filestr;

void Algae::initialize(void)
{
	LifeForm::add_creator(Algae::create, "Algae");
}

String Algae::species_name(void) const
{
	return "Algae";
}

String Algae::player_name(void) const
{
	return "Algae";
}

Algae::Algae()
{
	photo_event = new Event(algae_photo_time,
		make_procedure(*this, &Algae::photosynthesize));
}

void Algae::draw(int x, int y) const
{
#if DEBUG
	cout << "drawing Algae";
#endif /* DEBUG */
	win.draw_rectangle(x, y, x + 3, y + 3, true);
}

Color Algae::my_color(void) const
{
	return GREEN;
}

LifeForm* Algae::create(void)
{
	return new Algae;
}


Action Algae::encounter(const ObjInfo&)
{
	return LIFEFORM_IGNORE;
}

void Algae::photosynthesize(void)
{
	filestr << "At time " << Event::now() << " photosynthesize is called for " << this << " having energy " << energy << endl;
	photo_event = 0;
	energy += Algae_energy_gain;
	//if (energy > 2.0 * start_energy) {
	if (energy > 0.5*start_energy) {
		Algae* child = new Algae;
		reproduce(child);
	}
	photo_event = new Event(algae_photo_time,
		make_procedure(*this, &Algae::photosynthesize));
}

