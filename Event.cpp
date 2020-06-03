#include <iostream>
#include <cassert>
#include <algorithm>
#include <queue>
#include <vector>
#include "Params.h"
#include "Mem.h"
#include "Event.h"
#include <fstream>

using namespace std;
extern fstream filestr;
SimTime Event::_now = 0;

struct EventCompare {
	bool operator()(const Event* ep1, const Event* ep2) {
		return ep1->t > ep2->t;     // backwards on purpose so that events 
									// are processed in increasing order
	}
};

class PQueue {

public:
	//TODO: make it private again.
	vector<Event*> V;
	PQueue(void) {} // normal construction
	~PQueue(void);

	void insert(Event* e) {
		V.push_back(e);
		push_heap(V.begin(), V.end(), EventCompare());
		/*filestr<<" event inserted is "<<e<<endl;
		filestr<<" all events after one events' insertion are ";
		vector<Event*>::iterator it = (V).begin();
		for (it;it<(V).end();++it) {
		filestr<<*it<<" "<<(*it)->t<<"  ";
		}*/
		filestr << endl;
	}

	Event* pop_greatest(void) {
		pop_heap(V.begin(), V.end(), EventCompare());
		Event* e = V.back();
		V.pop_back();
		return e;
	}


	/*
	* remove an event from the queue
	*/
	void remove(const Event* e) {
		typedef vector<Event*>::iterator It;
		It i;

		int _count = 0;

		for (i = V.begin(); i < V.end(); i++) {
			if ((*i) == e) { break; }
			_count++;
		}

		//cout<<endl<<"The count of the deleted event in event queue is "<<_count<<endl;

		if ((*i) == e) {
			V.erase(i);
			make_heap(V.begin(), V.end(), EventCompare());
		}
	}

	/*
	* delete all of the events that operate on 'p'
	* NOTE: we don't have to remove the events from the queue when
	* we delete them since Event::~Event will do that for us
	*/
	void delete_matching(void* p) {
		typedef vector<Event*>::iterator It;

		bool done = false;
		while (!done) {
			done = true;
			for (It i = V.begin(); i < V.end(); i++) {
				if ((*i)->operates_on(p)) {
					delete *i;
					done = false;
					break;
				}
			}
		}
#if (DEBUG)
		for (It i = V.begin(); i < V.end(); i++)
			assert(!(*i)->operates_on(p));
#endif /* DEBUG */
	}

	unsigned size(void) const {
		return V.end() - V.begin();
	}
};

/* delete all of the events */
PQueue::~PQueue() {
	/* this look relies on the events removing themselves from the queue */
	/* it's slow, but it's simple (and who cares how fast it is,
	the program's over by now */
	while (!V.empty()) {
		Event* e = V.front();
		delete e;
	}
}

PQueue Event::equeue;
unsigned Event::nevents = 0;


Event::~Event()
{
	nevents -= 1;
	if (in_queue) remove();
}

/*
* simulate until there are no more events to simulate
*/
void Event::do_next(void)
{
	Event* e = equeue.pop_greatest();
	e->in_queue = false;
	assert(e->t >= _now);
	_now = e->t;
	/*#if DEBUG*/
	//cout << "doing event at time " << _now << endl;
	/* DEBUG */
	(*e)();
	delete e;
}

//TODO: remove the function 
void Event::printall()
{
	filestr << " all events as printed are ";
	vector<Event*>::iterator it = (equeue.V).begin();
	for (it; it<(equeue.V).end(); ++it) {
		filestr << *it << " " << (*it)->t << "  ";
	}
	filestr << endl;
};

unsigned Event::num_events(void)
{
	return equeue.size();
}

void Event::remove(void)
{
	assert(in_queue);
	equeue.remove(this);
	in_queue = 0;
}

void Event::insert()
{
	in_queue = true;
	assert(Event::_now <= t);
	equeue.insert(this);
}


void Event::delete_matching(void* p)
{
	equeue.delete_matching(p);
}
