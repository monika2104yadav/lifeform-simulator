#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include "Params.h"
#include "Event.h"
#include "String.h"
#include "ObjInfo.h"
#include "Set.h"
#include "Rock.h"
#include "Window.h"
extern fstream filestr;
using namespace std;

static Initializer<Rock> __Rock_initializer;

String Rock::species_name(void) const
{
	return "Rock";
}

Action Rock::encounter(const ObjInfo&)
{
	return LIFEFORM_IGNORE;
}

void Rock::initialize(void)
{
	LifeForm::add_creator(Rock::create, "Rock");
}


Rock::Rock(void)
{
}

Rock::~Rock(void)
{
	cout << "Rocks are still surviving" << endl;
}


Color Rock::my_color(void) const
{
	return ORANGE;
}

LifeForm* Rock::create(void)
{
	return new Rock;
}

