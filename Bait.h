#if !(_Bait_h)
#define _Bait_h 1


#include "LifeForm.h"
#include "Init.h"

class Bait : public LifeForm {

	static LifeForm* create(void);
	static void initialize(void);
	void birth(void);
	void wake_up(void);
	void hibernate(void);
	virtual void choose_course(ObjList&);
	virtual void fission(void);  //doesn't create a problem. as anyhow u have the definitions of both of these in the derived class.

public:

	Bait(void);
	virtual ~Bait(void);//not affecting after correction in construction of ShyBait because QuadTree has ShyBait instead of Bait pointer, so a non-virtual destructor would be fine.
	Color my_color(void) const;
	std::string species_name(void) const;
	std::string player_name(void) const;
	Action encounter(const ObjInfo&);
	friend class Initializer<Bait>;
};

class ShyBait : public Bait {
	/*CHECK IF THE TWO FUNCTIONS BELOW ARE COMMENTED OUT THEN THE PROGRAM SHOULD CRASH*/
	// virtual void choose_course(ObjList& i);
	// virtual void fission(void);
public:
	ShyBait();
	~ShyBait();
};

#endif /* !(_Bait_h) */

