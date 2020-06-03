#if ! (_LifeForm_h)
#define _LifeForm_h 1

#include <cassert>
#include <vector>
#include <map>
#include <algorithm>
#ifdef _MSC_VER
#include <time.h>
#else
#include <sys/time.h>
#endif
#include "Params.h"
#include "Point.h"


/* forward declarations */
class LifeForm;
class istream;
struct ObjInfo;
template <class T> class Set;
typedef std::vector<ObjInfo> ObjList;
template <class Obj> class QuadTree;
template <class Obj> class TreeNode;

/*
* The map will contain IstreamCreators for LifeForms
* The map will be keyed on a String.  This String
* must be a keyword that uniquely identifies a type of LifeForm
* (e.g., "Lion", "Tiger", "Bear", oh my).
*/
typedef LifeForm* (*IstreamCreator)(void);

typedef std::map<std::string, IstreamCreator> LFCreatorTable;

/*
* The Canvas class is something we can draw on
*/
class Canvas;

/*
* We draw with Colors
*/
#include "Color.h"

/*
* The Region class represents the simulation space (2-dimensional)
*/
class Event;


enum Action {
	LIFEFORM_IGNORE,
	LIFEFORM_EAT
};

class LifeForm {
	static LifeForm* all_life;    // a doubly-linked list of all life forms
	static QuadTree<LifeForm*> space;
	LifeForm* next_life;          // links for that list
	LifeForm* prev_life;
	static unsigned num_life;     // length of the list

	static LFCreatorTable& istream_creators(void);
	int scale_x(double) const;
	int scale_y(double) const;

	double energy;
	bool is_alive;

	Event* border_cross_event;    // pointer to the event for the next
								  // encounter with a boundary
	void border_cross(void);

	void region_resize(void);

	Point pos;
	double update_time;           // the time when update_position was 
								  //   last called
	double reproduce_time;        // the time when reproduce was last called
	void resolve_encounter(LifeForm*);
	void print_position(void) const;
	void print(void) const;
	void eat(LifeForm*);
	void age(void);               // subtract age_penalty from energy
	void gain_energy(double);
	void update_position(void);   // calculate the current position for
								  // an object.  If less than Time::tolerance
								  // time units have passed since the last
								  // call to update_position, then do nothing
								  // (we can't have moved very far so there's
								  // no point in updating our position)

	void check_encounter(void);   // check to see if there's another object
								  // within encounter_distance.  If there's
								  // an object nearby, invoke resove_encounter
								  // on ourself with the closest object

	void die(bool = true);          // life form must be in the quad tree.

									/* !!! WARNING methods die and _die delete 'this'
									use with EXTREME care */
	void _die(void);
	void compute_next_move(void);

	ObjInfo info_about_them(const LifeForm*) const;

	double course;
	double speed;

	Point start_point;
	const Point& position() const { return pos; }

protected:
	static Canvas win;
	double health(void) const {
		return energy / start_energy;
	}
	void set_course(double);
	void set_speed(double);
	double get_course(void) const { return course; }
	double get_speed(void) const { return speed; }
	void reproduce(LifeForm*);
	ObjList perceive(double);

public:
	LifeForm(void);
	virtual ~LifeForm(void);

	static void add_creator(IstreamCreator, const std::string&);
	static void create_life();
	/* draw the lifeform on 'win' where x,y is upper left corner */
	virtual void draw(int, int) const;
	virtual Color my_color(void) const = 0;

	void display(void) const;
	static void redisplay_all(void);
	static void clear_screen(void);

	virtual Action encounter(const ObjInfo&) = 0;
	virtual std::string species_name(void) const = 0;
	virtual std::string player_name(void) const;

	friend class Algae;

	/*
	* the following functions are used for testing only and should not be used by students (except, of course,
	* during testing)
	*/
	bool confirmPosition(double xpos, double ypos) {
		return pos.distance(Point(xpos, ypos)) < 0.10;
	}

	static void runTests();
	static bool testMode;
	static void place(LifeForm*, Point p);

};

#endif /* !(_LifeForm_h) */

