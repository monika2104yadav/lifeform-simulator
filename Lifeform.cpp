/*
* LifeForm.cc -- Merged with Lifeform-Craig.cc to provide
*                complete implementation of Lifeform base.
* See LifeForm.h for details (and the project instructions)
*
* 7/6/2012
*/
#include <iostream>
#include <fstream>
#include "CraigUtils.h"
#include "Window.h"
#include "tokens.h"
#include "Procedure.h"
#include "ObjInfo.h"
#include "QuadTree.h" 
#include "Params.h"
#include "LifeForm.h"
#include "Event.h"
#include "String.h"

using namespace std;
double MAX_SIMULATION_TIME = 50000.0;
extern fstream filestr;
/*
* Implementation NOTE:
* Here's a cute trick.......at
* we want to make *SURE* that the istream_creators hash table is
* constructed *BEFORE* we try to insert anything into it....
* But HOW can we force that to happen?  Well, instead of making
* istream_creators a static data member, I make it a static member
* function (returning a reference to the table).
* the *REAL* hash table is a static local variable in this member
* function.
* So... when add_creator is called, it will call istream_creators()
* and 'the_real_table' will be constructed before anything is
* actually inserted.
*/
LFCreatorTable& LifeForm::istream_creators()
{
	static LFCreatorTable the_real_table;
	return the_real_table;
}

QuadTree<LifeForm*> LifeForm::space(0.0, 0.0,
	grid_max, grid_max);
Canvas LifeForm::win(win_x_size, win_y_size);

LifeForm* LifeForm::all_life = 0;
unsigned LifeForm::num_life = 0;

LifeForm::LifeForm(void)
{
	energy = start_energy;
	course = speed = 0.0;
	pos = Point(0, 0);
	is_alive = false;
	update_time = Event::now();
	reproduce_time = 0.0;
	border_cross_event = Nil<Event>();

	/* insert into all_life list */
	next_life = all_life;
	if (all_life) all_life->prev_life = this;
	prev_life = Nil<LifeForm>();
	all_life = this;
	num_life += 1;
}

LifeForm::~LifeForm(void)
{
	assert(!is_alive);
	if (next_life) next_life->prev_life = prev_life;
	if (prev_life) prev_life->next_life = next_life;
	if (all_life == this)
		all_life = next_life;
	num_life -= 1;
}

void LifeForm::die(bool in_tree)
{
	filestr << "At time " << Event::now() << " " << "At pos (" << pos.xpos << ", " << pos.ypos << ") " << "die called for the specie " << species_name() << " with " << this << " having energy " << energy << endl;
	if (!is_alive) return;
	is_alive = false;
	if (in_tree) space.remove(pos);
	filestr << " the value of the event is " << new Event(0.0, make_procedure(*this, &LifeForm::_die));
}

void LifeForm::_die(void)
{
	Event::delete_matching(this);
	delete this;
}

String LifeForm::player_name(void) const
{
	return species_name();
}

void LifeForm::age(void)
{
	filestr << "At time " << Event::now() << " " << "At pos (" << pos.xpos << ", " << pos.ypos << ") " << "age called for the specie " << species_name() << " with " << this << " having energy " << energy << endl;
	if (!is_alive) return;
	energy = energy - age_penalty;
	if (energy < min_energy) {
		die();
		return;
	}
	filestr << " the value of the event is " << new Event(age_frequency, make_procedure(*this, &LifeForm::age));
}

void LifeForm::gain_energy(double energy_value)
{
	filestr << "At time " << Event::now() << " " << "At pos (" << pos.xpos << ", " << pos.ypos << ") " << "gain energy called for the specie " << species_name() << " with " << this << " having energy " << energy << " with added energy " << energy_value << endl;
	if (!is_alive) return;
	energy += energy_value;
}

void LifeForm::compute_next_move(void)
{
	filestr << "At time " << Event::now() << " " << "At pos (" << pos.xpos << ", " << pos.ypos << ") " << "compute next move called for the specie " << species_name() << " with " << this << " having energy " << energy << endl;
	if (!is_alive) return;
	if (border_cross_event != Nil<Event>()) {
		filestr << "border cross event deleted " << endl;
		delete border_cross_event;
		border_cross_event = Nil<Event>();
	}
	double edge_d = space.distance_to_edge(pos, course);
	double cross_t = 0.0;
	if (speed > Point::tolerance) {
		cross_t = edge_d / speed + Point::tolerance;
		border_cross_event = new Event(cross_t,
			make_procedure(*this, &LifeForm::border_cross));
		filestr << " the value of the event is " << border_cross_event << endl;
		filestr << " after time " << cross_t
			<< " for border at distance " << edge_d << " at speed " << speed << endl;
	}
}

void LifeForm::border_cross(void)
{
	filestr << "At time " << Event::now() << " " << "At pos (" << pos.xpos << ", " << pos.ypos << ") " << "border cross called for the specie " << species_name() << " with " << this << " having energy " << energy << endl;
	if (!is_alive) return;
	border_cross_event = Nil<Event>();
	update_position();
	if (!is_alive) return;
	check_encounter();
	if (!is_alive) return;
	compute_next_move();
}

void LifeForm::check_encounter(void)
{
	filestr << "At time " << Event::now() << " " << "At pos (" << pos.xpos << ", " << pos.ypos << ") " << "check encounter called for the specie " << species_name() << " with " << this << " having energy " << energy << endl;
	static int count = 1;
	if (!is_alive) return;
	count++;

	vector<LifeForm*> prey_list = space.nearby(pos, encounter_distance);
	filestr << " the prey list includes the following " << endl;
	for (vector<LifeForm*>::iterator it = prey_list.begin(); it < prey_list.end(); ++it)
		filestr << ((*it)->species_name()) << endl;

	if (prey_list.size() == 0)  return;
	resolve_encounter(prey_list.front());
}

void LifeForm::resolve_encounter(LifeForm* enc)
{
	filestr << "At time " << Event::now() << " " << "At pos (" << pos.xpos << ", " << pos.ypos << ") " << "resolve encounter called for the specie " << species_name() << " with " << this << " having energy " << energy << endl;
	if (enc == Nil<LifeForm>()) return;
	if (!(enc->is_alive)) return;
	if (!is_alive) return;
	energy -= encounter_penalty;
	enc->energy -= encounter_penalty;
	if (energy < min_energy || enc->energy < min_energy) {
		if (energy < min_energy) die();
		if (enc->energy < min_energy) enc->die();
		return;
	}
	double rand_n = drand48() + 0.0;
	double eat_enc = eat_success_chance(energy, enc->energy);
	double eat_this = eat_success_chance(enc->energy, energy);

	/* case where the object and encountered object are not heading towards each other */
	//double tri_two_angles = fmod(abs(course), M_PI) + 
	//fmod(abs(enc->get_course()), M_PI);

	/*double collision_bool = fmod((course + enc->get_course()),M_PI/2);
	if (!(collision_bool == Point::tolerance) )
	{
	switch(encounter(info_about_them(enc)))
	{
	case LIFEFORM_EAT:
	if (rand_n < eat_enc) eat(enc);
	break;
	case LIFEFORM_IGNORE:
	break;
	}
	if (energy < min_energy) die();
	return;
	}*/
	/* case where there is a head on collision between two objects */
	switch (encounter(info_about_them(enc)))
	{
	case LIFEFORM_EAT:
		switch (enc->encounter(info_about_them(this)))
		{
		case LIFEFORM_IGNORE:
			if (rand_n < eat_enc) eat(enc);
			break;
		case LIFEFORM_EAT:
			if (rand_n < eat_enc && rand_n < eat_this)
			{
				switch (encounter_strategy) {
				case FASTER_GUY_WINS: //more speed 
					if (enc->speed < this->speed) eat(enc);
					else enc->eat(this);
					break;
				case SLOWER_GUY_WINS: //less speed
					if (speed < enc->speed) eat(enc);
					else enc->eat(this);
					break;
				case EVEN_MONEY: //flip a coin
					if (ceil(drand48() * 100) > 50)
						enc->eat(this);
					else eat(enc);
					break;
				case BIG_GUY_WINS: //more energy
					if (enc->energy < energy) eat(enc);
					else enc->eat(this);
					break;
				case UNDERDOG_IS_HERE: //less energy
					if (energy < enc->energy) eat(enc);
					else enc->eat(this);
					break;
				}
			}
			if (rand_n < eat_enc && rand_n > eat_this) eat(enc);
			if (rand_n > eat_enc && rand_n < eat_this) enc->eat(this);
			break;
		}
		break;
	case LIFEFORM_IGNORE:
		switch (enc->encounter(info_about_them(this)))
		{
		case LIFEFORM_IGNORE:
			break;
		case LIFEFORM_EAT:
			if (rand_n < eat_this) enc->eat(this);
			break;
		}
		break;
	}
}

void LifeForm::eat(LifeForm* prey)
{
	filestr << "At time " << Event::now() << " " << "At pos (" << pos.xpos << ", " << pos.ypos << ") " << "eat called for the specie " << species_name() << " with " << this << " having energy " << energy << endl;
	if (prey == Nil<LifeForm>()) return;
	if (!is_alive || !(prey->is_alive)) return;
	prey->die();
	filestr << "does it reach here after prey's death!! " << endl;
	energy -= eat_cost_function(energy, prey->energy);
	if (energy < min_energy) {
		die();
		return;
	}
	filestr << "does it reach here to set gain energy!!! " << endl;
	double next_t = Event::now() + digestion_time;
	double gain_e = prey->energy * eat_efficiency;
	filestr << "the next gain event is kept at time " << next_t << " with the value of gain_energy " << gain_e << " and member function pointer " << &LifeForm::gain_energy << endl;
	filestr << " the value of the event is " << new Event(digestion_time, (make_procedure<LifeForm, double>(*this, (&LifeForm::gain_energy))).bind(gain_e)) << endl;

}

void LifeForm::update_position(void)
{
	filestr << "At time " << Event::now() << " " << "At pos (" << pos.xpos << ", " << pos.ypos << ") " << "update position called for the specie " << species_name() << " with " << this << " having energy " << energy << endl;
	if (!is_alive) return;
	double last_t = Event::now() - update_time;
	double tol_unit = 0.0001;
	if (last_t < tol_unit) return;

	double f_xpos = (speed*last_t*((cos(course)))) + pos.xpos;
	double f_ypos = (speed*last_t*((sin(course)))) + pos.ypos;

	double l_grid_min = grid_max / 4.0 - Point::tolerance;
	double l_grid_max = grid_max / 2.0 + grid_max / 4.0 + Point::tolerance;
	if (species_name() == "Craig" || species_name() == "Algae") {
		l_grid_min = 0;
		l_grid_max = grid_max;
	}
	if ((f_xpos < l_grid_min || f_xpos > l_grid_max) ||
		(f_ypos < l_grid_min || f_ypos > l_grid_max)) {
		die();
		return;
	}
	energy -= movement_cost(speed, last_t);
	if (energy < min_energy) {
		die();
		return;
	}
	update_time = Event::now();
	Point f_pos(f_xpos, f_ypos);
	space.update_position(pos, f_pos);
	pos.xpos = f_xpos;
	pos.ypos = f_ypos;
}

void LifeForm::region_resize(void)
{
	filestr << "At time " << Event::now() << " " << "At pos (" << pos.xpos << ", " << pos.ypos << ") " << "region resize called for the specie " << species_name() << " with " << this << " having energy " << energy << endl;
	if (!is_alive) return;
	update_position();
	if (!is_alive) return;
	compute_next_move();
}

ObjInfo LifeForm::info_about_them(const LifeForm* life) const
{
	/* calc. of bearing */
	filestr << "At time " << Event::now() << " " << "At pos (" << pos.xpos << ", " << pos.ypos << ") " << "info about them called for the specie " << species_name() << " with " << this << " having energy " << energy << endl;
	double dist_d = sqrt(pow((life->pos.xpos - pos.xpos), 2) +
		pow((life->pos.ypos - pos.ypos), 2));
	double bearing = 0.0;
	double xpos_diff = life->pos.xpos - pos.xpos;
	double del_x = life->pos.xpos - pos.xpos;
	double del_y = life->pos.ypos - pos.ypos;
	if (fabs(del_y) <= fabs(1.0e-10 * del_x)) { // delta y is essentially 0
		if (del_x > 0.0) bearing = 0;
		else bearing = M_PI;
	}
	else { /* atan returns the correct result if delta x is positive */
		if (del_x > 0.0)
			bearing = atan(del_y / del_x);
		else
			bearing = atan(del_y / del_x) + M_PI;
	}
	ObjInfo life_info;
	life_info.species = life->species_name();
	life_info.health = life->health();
	life_info.distance = dist_d;
	life_info.bearing = bearing;
	life_info.their_course = life->get_course();
	life_info.their_speed = life->get_speed();
	return life_info;
}

/*** description of all protected member functions follows ***/
void LifeForm::set_course(double new_course)
{
	filestr << "At time " << Event::now() << " " << "At pos (" << pos.xpos << ", " << pos.ypos << ") " << "set course called for the specie " << species_name() << " with " << this << " having energy " << energy << endl;
	if (!is_alive) return;
	update_position();
	if (!is_alive) return;
	course = new_course;
	compute_next_move();
}

void LifeForm::reproduce(LifeForm* new_life)
{
	filestr << "At time " << Event::now() << " " << "At pos (" << pos.xpos << ", " << pos.ypos << ") " << "reproduce called for the specie " << species_name() << " with " << this << " having energy " << energy << endl;
	new_life->is_alive = true;
	double dist_others = 0.0;
	int count = 0;
	double lgrid_max = grid_max / 2.0 + grid_max / 4.0 + Point::tolerance;
	double lgrid_min = grid_max / 4.0 - Point::tolerance;
	if (species_name() == "Craig" || species_name() == "Algae") {
		lgrid_min = 0;
		lgrid_max = grid_max;
	}
	LifeForm* l_closest = Nil<LifeForm>();
	double dist_parent = HUGE;
	while ((dist_others < (encounter_distance + Point::tolerance)) && count < 5)
	{
		do {
			new_life->pos.xpos = drand48()*reproduce_dist + pos.xpos;
			new_life->pos.ypos = drand48()*reproduce_dist + pos.ypos;
			dist_parent = sqrt(pow((new_life->pos.xpos - pos.xpos), 2) +
				pow((new_life->pos.ypos - pos.ypos), 2));
		} while ((new_life->pos.xpos > lgrid_max || new_life->pos.xpos < lgrid_min
			|| new_life->pos.ypos > lgrid_max || new_life->pos.ypos < lgrid_min
			|| dist_parent < (encounter_distance + Point::tolerance)));
		LifeForm* l_closest = space.closest(new_life->pos);
		dist_others = sqrt(pow((new_life->pos.xpos - l_closest->pos.xpos), 2) +
			pow((new_life->pos.ypos - l_closest->pos.ypos), 2));
		count++;
	}
	if (dist_others < (encounter_distance + Point::tolerance)) {
		filestr << " the value of the event is " << new Event(0.0, (make_procedure<LifeForm, LifeForm*>(*new_life, (&LifeForm::resolve_encounter))).bind(l_closest));
	}
	new_life->start_point = new_life->pos;
	space.insert(new_life, new_life->pos,
		make_procedure(*new_life, &LifeForm::region_resize));
	filestr << " the value of the event is " << new Event(age_frequency, make_procedure(*new_life, &LifeForm::age));

	/*checks included later to enable die() for new_life*/
	if (this->is_alive == false) {
		new_life->die();
		return;
	}
	double lrep_diff = Event::now() - reproduce_time;
	if (lrep_diff < min_reproduce_time + Point::tolerance) {
		new_life->die();
		return;
	}

	new_life->energy = (energy / 2) * (1 - reproduce_cost);
	energy = (energy / 2) * (1 - reproduce_cost);
	if (energy < min_energy) {
		die();
		new_life->die();
		return;
	}
	reproduce_time = Event::now();
	new_life->reproduce_time = Event::now();
}

ObjList LifeForm::perceive(double dist)
{
	filestr << "At time " << Event::now() << " " << "At pos (" << pos.xpos << ", " << pos.ypos << ") " << "perceive called for the specie " << species_name() << " with " << this << " having energy " << energy << endl;
	if (dist > max_perceive_range) dist = max_perceive_range;
	if (dist < min_perceive_range) dist = min_perceive_range;
	ObjList r_vector;
	if (!is_alive) return r_vector;
	energy -= perceive_cost(dist);
	if (energy < min_energy) {
		die();
		return r_vector;
	}
	std::vector<LifeForm*> near_l = space.nearby(pos, dist);
	std::vector<LifeForm*>::iterator it = near_l.begin();
	for (it; it != near_l.end(); it++) {
		LifeForm* lifeform = *it;
		r_vector.push_back(this->info_about_them(lifeform));
	}
	return r_vector;
}

void LifeForm::set_speed(double new_speed)
{
	filestr << "At time " << Event::now() << " " << "At pos (" << pos.xpos << ", " << pos.ypos << ") " << "set speed called for the specie " << species_name() << " with " << this << " having energy " << energy << endl;
	if (!is_alive) return;
	if (new_speed > max_speed) return;
	update_position();
	if (!is_alive) return;
	speed = new_speed;
	compute_next_move();
}

/*** description of all public member functions follows ***/
void LifeForm::add_creator(IstreamCreator f, const std::string& s)
{
	(istream_creators())[s] = f;
}

int LifeForm::scale_x(double x) const
{
	double rel_x = (double)x / (double)grid_max;
	return (int)(rel_x * win_x_size);
}

int	LifeForm::scale_y(double y) const
{
	double rel_y = (double)y / (double)grid_max;
	return (int)(rel_y * win_y_size);
}

void LifeForm::print(void) const
{
	print_position();
}

void LifeForm::print_position(void) const
{
}

void LifeForm::create_life()
{

	LifeForm* obj;
	if (testMode) { runTests(); return; }

	String x;
	int yylex(void);
	extern FILE* yyin;
	yyin = fopen("config.test", "r");
	extern String lex_text;
	extern int lex_int;
	Token tok;
	tok = (Token)yylex();
	while (tok != TOK_EOF)
	{
#ifdef DEBUG
		cout << "token is " << tok << endl;
#endif 
		IstreamCreator factory_fun = (istream_creators())[lex_text];
		tok = (Token)yylex();
		for (int i = 0; i < lex_int; i++) {
			obj = factory_fun();
			/*String derived_name = typeid(*obj).name();
			size_t space_pos = derived_name.rfind(" ");
			size_t end_pos = derived_name.size();
			String class_name = derived_name.substr(space_pos+1, end_pos);
			if (!(class_name == lex_text)) {
			delete obj;//i am not sure if something is returned as a static then how far this could be true.
			factory_fun = (istream_creators())[class_name];
			obj = factory_fun();
			}*/
			LifeForm* nearest;
			do {
				obj->pos.ypos = drand48() * grid_max / 2.0 + grid_max / 4.0;
				obj->pos.xpos = drand48() * grid_max / 2.0 + grid_max / 4.0;
				if (num_life > 1)
					nearest = space.closest(obj->pos);
				else
					nearest = Nil<LifeForm>();
			} while (nearest && nearest->position().distance(obj->position())
				<= encounter_distance);
			obj->start_point = obj->pos;
			space.insert(obj, obj->pos, make_procedure(*obj, &LifeForm::region_resize));

			filestr << " the value of the event is " << new Event(age_frequency, make_procedure(*obj, &LifeForm::age)) << endl;
			obj->is_alive = true;
		}
		tok = (Token)yylex();
	}
	redisplay_all();
}

void LifeForm::display(void) const
{
#ifdef DEBUG
	cout << "drawing LF at";
	cout << "(" << pos.xpos << "," << pos.ypos << ")";
#endif /* DEBUG */
	win.set_color(my_color());
	draw(scale_x(pos.xpos), scale_y(pos.ypos));
#ifdef DEBUG
	//filestr<<"At time "<<Event::now()<<" "<<"At pos ("<<pos.xpos<<", "<<pos.ypos<<") " << endl;
#endif /* DEBUG */
}

typedef pair<String, double> Rank;
struct RankCompare {
	bool operator()(const Rank& x, const Rank& y) {
		return x.second > y.second;
	}
};


void LifeForm::redisplay_all(void)
{
	//for (int i = 0; i < 100; i++) {
	cout << "check the correctness of event simulation" << endl;
	typedef map<String, double> SpeciesHT;
	SpeciesHT species_table;
	static int max_species = 0;
	//Event::printall();
	win.clear();
	for (LifeForm* k = all_life; k; k = k->next_life) {
		if (k->is_alive) {
			/* uncomment the next line to get accurate graphics
			at the expense of slowing down the simulator */
			/* k->update_position();*/
			k->display();
			String name = k->player_name();
			name = name.substr(0, name.find(':'));
			if (species_table.find(name) == species_table.end()) {
				species_table[name] = 0.0;
			}
			species_table[name] += k->energy;
		}
	}
	win.flush();

#if (SPECIES_SUMMARY)
	cout << "\n\n\n";
	cout << "At Time " << Event::now() << " there are " << num_life << " total life forms, ";
	int count = 0;
	vector<Rank> rankings;
	typedef vector<Rank>::iterator Set_IT;
	for (SpeciesHT::iterator i = species_table.begin(); i != species_table.end(); ++i)
	{
		rankings.push_back(Rank(i->first, i->second));
		count += 1;
	}
	cout << "and " << count << " distinct species\n";
	cout << "There are " << Event::num_events() << " events (" << (double)Event::num_events()
		/ (double)num_life << " events per life form)\n";

	if (count > max_species) max_species = count;
	sort(rankings.begin(), rankings.end(), RankCompare());
	int specs = 0;

	for (vector<Rank>::iterator i = rankings.begin(); i != rankings.end(); ++i) {
		Rank& best = *i;
		//filestr<<"At time "<<Event::now()<<" "<<"At pos ("<<pos.xpos<<", "<<pos.ypos<<") " << "Species: " << best.first << " has total of " << best.second << " energy\n";
		specs += 1;
		if (specs >= max_species / 2) {  // draw line to indicate winners/losers
			cout << "----------------------------------------";
			cout << "----------------------------------------\n";
			specs = -specs; // make sure the line is drawn only once
		}
	}

	if (termination_strategy == RUN_TILL_HALF_EXTINCT && count <= max_species / 2
		|| termination_strategy == RUN_TILL_ONE_SPECIES_LEFT && count <= 2
		|| Event::now() > MAX_SIMULATION_TIME) {
		cout << "\t!!Simulation Complete at time " << Event::now() << " !!\n";
		//cout << "hit CTRL-C to stop\n";  // uncomment if you want to see the
		//sleep(1000);                     // final state of the graphics display
		exit(0);
	}
#endif /* SPECIES_SUMMARY */
}

void LifeForm::draw(int x, int y) const
{
	win.draw_rectangle(x, y, x + 4, y + 4);
}

void LifeForm::clear_screen(void)
{
	win.clear();
}

/*void LifeForm::runTests()
{
}

void LifeForm::place(LifeForm* obj, Point p)
{
obj->start_point = p;
obj->pos = p;
space.insert(obj, obj->pos, make_procedure(*obj, &LifeForm::region_resize));
(void) new Event(age_frequency, make_procedure(*obj, &LifeForm::age));
obj->is_alive = true;
}
*/