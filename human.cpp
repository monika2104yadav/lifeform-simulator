#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include "CraigUtils.h"
#include "String.h"
#include "Params.h"
#include "Event.h"
#include "ObjInfo.h"
#include "Set.h"
#include "human.h"
#include "Window.h"

using namespace std;
extern fstream filestr;

Initializer<Human> __Human_initializer;

String Human::species_name(void) const
{
	return species;
}
//TODO: make changes in .h for these.
String Human::player_name()
{
	return "Monika";
}

Color Human::my_color(void) const
{
	return CYAN;
}

Action Human::encounter(const ObjInfo& info)
{
	//filestr<<"encounter called for Human"<<endl;
	bool b_check = check_same_specie(info);
	if (b_check) {
		set_course(info.bearing + M_PI);
		encode_specie_name();
		return LIFEFORM_IGNORE;
	}
	else
	{
		set_speed((info).their_speed + 1.0*drand48());
		//to help decide for the next best move.
		Delete(choose_course_event);
		choose_course_event = new Event(0.0,
			make_procedure(*this, &Human::choose_course));

		double prey_health = info.health;
		if (health() >= 2 * prey_health) {
			Human* child = new Human;
			reproduce(&(*child));
			child->set_speed(this->get_speed());
			double tri_two_angles = fmod(abs(info.their_course), M_PI) +
				fmod(abs(this->get_course()), M_PI);
			if (tri_two_angles > M_PI + Point::tolerance) {
				//means the enemy is not directed towards the parent.
				child->set_course(info.their_course + M_PI);
			}
			else {
				child->set_course(info.their_course);
			}
			child->encode_specie_name();
			//filestr<<"reproduction by the Human"<<endl;
		}
		encode_specie_name();
		return LIFEFORM_EAT;
	}
}

void Human::initialize(void)
{
	LifeForm::add_creator(Human::create, "Human");
}

LifeForm* Human::create(void)
{
	Human* res = Nil<Human>();
	res = new Human();
	res->display();
	return res;
}

Human::Human()
{
	perceive_radius = 5;
	choose_course_event = Nil<Event>();
	(void) new Event(0.0,
		make_procedure(*this, &Human::live));
}

Human::~Human()
{
	//filestr<<"Humans destructor"<<endl;
}

void Human::live()
{
	set_course(drand48() * 2.0 * M_PI);
	set_speed(2 + 5.0*drand48());
	(void) new Event(0.0,
		make_procedure(*this, &Human::choose_course));
	species = "my4374:Human";
	encode_specie_name();
}

/* the return value is the median distance of the species
from the human*/
double Human::measure_spontaneity(ObjList& prey_list)
{
	int num_quad1 = 0;	int num_quad2 = 0;	int num_quad3 = 0;	int num_quad4 = 0;
	vector<ObjInfo> quad_elem1, quad_elem2, quad_elem3, quad_elem4;

	for (ObjList::iterator i = prey_list.begin(); i != prey_list.end(); ++i)
	{
		if (check_same_specie((*i)) == true)
			continue;
		double bearing = (*i).bearing;
		double cos_bearing = cos(bearing);
		double sin_bearing = sin(bearing);

		if (cos_bearing > 0 && sin_bearing > 0) {
			++num_quad1;
			quad_elem1.push_back(*i);
		}
		else if (cos_bearing < 0 && sin_bearing > 0) {
			++num_quad2;
			quad_elem2.push_back(*i);
		}
		else if (cos_bearing < 0 && sin_bearing < 0) {
			++num_quad3;
			quad_elem3.push_back(*i);
		}
		else if (cos_bearing > 0 && sin_bearing < 0) {
			++num_quad4;
			quad_elem4.push_back(*i);
		}
	}
	int ret_radius = 0;
	if (num_quad1 > num_quad2 && num_quad1 > num_quad3 && num_quad1 > num_quad4)
		ret_radius = quad_set_course_speed(quad_elem1);
	if (num_quad2 > num_quad1 && num_quad2 > num_quad3 && num_quad2 > num_quad4)
		ret_radius = quad_set_course_speed(quad_elem2);
	if (num_quad3 > num_quad1 && num_quad3 > num_quad2 && num_quad3 > num_quad4)
		ret_radius = quad_set_course_speed(quad_elem3);
	if (num_quad4 > num_quad1 && num_quad4 > num_quad2 && num_quad4 > num_quad3)
		ret_radius = quad_set_course_speed(quad_elem4);
	return ret_radius;
}


void Human::choose_course()
{
	choose_course_event = Nil<Event>();
	ObjList prey_list = perceive(perceive_radius);
	if (prey_list.size() == 0) {
		//In this case the percieve_radius should better get maximum. An increment can be
		//thought of as again inversely proportional to the crowd present. Note if it yielded
		//to be zero or 1 then it automatically sets to 
		//the initial value is going to be ( max_percieve_range - min_perceive_range ) / 2
		if (perceive_radius < max_perceive_range)
			perceive_radius += 2.5;
		//perceive_radius = max_perceive_range;
		//keeping slow speed.
		if (perceive_radius == max_perceive_range - Point::tolerance)
			set_speed(1.0*drand48() + 0.1);
		else
			set_speed(0.0);
		//encode_specie_name();
		//choose_course_event = new Event(0.0,
		//  make_procedure(*this, &Human::choose_course));
		//return;
	}
	else {
		//Note that the decrement can be made inversely proportional to the crowd present. 
		//greater the crowd greater could be the decrease in perceive_radius just with the
		//purpose of saving energy. So, if there are 4 species around and in total 12 
		//detected then the average distance of the species from the specie could be the 
		//next perceive radius. It would be in range and the only possibility if that is 
		//less than min_perceive_range. in that case the base class can automatically 
		//take care of it. better than the average would be the median of the distance 
		//could be the next percieve range. What can be a valid question is that if 
		//are many human beings then should they contribute to the average perceive_range
		//my answer to that question is that if they are then it is better to ignore
		//them probably. Because that will enable for a search higher to find a non-human
		//being. There is no smoothness in the change of percieve radius i can say 
		//abrupt things are observed and performance decreases. A better means for change
		//of factor 2.5 can't be known because the no. of species detected can't determine
		//the incremental change as that won't be smooth either!!! it is very necessary to keep that smooth as it may otherwise go to a wrong quadrant. also, the factor 2.5 should be preferably kept constant.
		if (perceive_radius > min_perceive_range + 2.5)
			perceive_radius -= 2.5;
		//perceive_radius = 
		measure_spontaneity(prey_list);
	}
	encode_specie_name();
	double t_next = perceive_radius - min_perceive_range;
	if (t_next < 0)
		t_next = 0.0;
	choose_course_event = new Event(t_next,
		make_procedure(*this, &Human::choose_course));
	//filestr<<"the value of speed for humans is detected as "<<get_speed()<<endl;
}

String float_to_string(double value)
{
	char* s_temp = new char[100];

	int inc_value = floor(value * 1000);
	String return_str = itoa(inc_value, s_temp, 10);
	if (return_str.size() == 0)
		return_str = "0000";
	else if (return_str.size() < 4) {
		while (return_str.size() < 4) {
			return_str.insert(0, "0");
		}
	}
	string::iterator retstr_it = return_str.end();
	advance(retstr_it, -3);
	return_str.insert(retstr_it, '.');

	delete s_temp;
	s_temp = NULL;
	return return_str;
}

void Human::encode_specie_name()
{
	String& s = species;
	double this_health = health();
	const String t_s = "time ";
	size_t time_pos = s.find(t_s);

	/*encoding time*/
	String time_string = float_to_string(Event::now());
	String::iterator time_it = time_string.end();

	if (time_pos > s.size()) {
		s.append(" time ");
	}
	else {
		s.erase(time_pos + t_s.size());
	}
	s.append(time_string);

	/*encoding health*/
	const String c_s = " health ";
	size_t health_pos = s.find(c_s);
	String health_string = float_to_string(health());
	if (health_pos > s.size()) {
		s.append(" health ");
	}
	s.append(health_string);
}

void Human::decode_specie_name(String input, double& health, double& time)
{
	const String t_s = "time ";
	size_t time_pos = input.find(t_s);
	const String c_s = " health ";
	size_t health_pos = input.find(c_s);

	/*decoding time*/
	time = atof((input.substr(time_pos, health_pos)).c_str());

	/*decoding health*/
	health = atof((input.substr(health_pos)).c_str());
}

bool Human::check_same_specie(const ObjInfo& info)
{
	double enc_health = info.health;
	double this_health = health();
	double time_now = Event::now();

	double enc_prev_health = 0.0;//parameters to be found from species_name encoding.
	double this_prev_health = 0.0;
	double this_prev_time = 0.0;
	double enc_prev_time = 0.0;

	const String c_s = " health ";

	String s = info.species;
	size_t f_pos = s.find(c_s);
	if (f_pos > s.size() || f_pos < 1)
		return false;

	decode_specie_name(info.species, enc_prev_health, enc_prev_time);
	decode_specie_name(species, this_prev_health, this_prev_time);
	double health_variation = (this_health - this_prev_health) / (time_now - this_prev_time);
	double max_expected_enc_health = 0.0;
	double min_expected_enc_health = 0.0;
	if (health_variation > 0) {
		max_expected_enc_health = enc_prev_health + (time_now - enc_prev_time)*health_variation;
		min_expected_enc_health = enc_prev_health - (time_now - enc_prev_time)*health_variation;
	}
	else {
		max_expected_enc_health = enc_prev_health - (time_now - enc_prev_time)*health_variation;
		min_expected_enc_health = enc_prev_health + (time_now - enc_prev_time)*health_variation;
	}
	int max_health_for_comparison = floor(max_expected_enc_health * 100) + 1;
	int min_health_for_comparison = floor(min_expected_enc_health * 100) - 1;
	int enc_health_for_comparison = floor(info.health * 100);

	if (enc_health_for_comparison < min_health_for_comparison ||
		enc_health_for_comparison > max_health_for_comparison) {
		return false;
	}
	return true;
}

double Human::quad_set_course_speed(vector<ObjInfo>& v)
{
	vector<ObjInfo>::iterator it;
	double best_d = HUGE;
	double ret_radius = 0;
	for (it = v.begin(); it < v.end(); ++it) {
		double it_health = (*it).health;
		ret_radius += (*it).distance;
		if (best_d >(*it).distance) {
			best_d = (*it).distance;
			set_course(((*it).bearing));
			set_speed((*it).their_speed + 1.0*drand48());
		}
	}
	ret_radius /= v.size();
	return ret_radius;
}