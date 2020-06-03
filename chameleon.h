#if !(_Craig_h)
#define _Craig_h 1

#include "LifeForm.h"
#include "Init.h"
#include "String.h"

class Chameleon : public LifeForm {
protected:
	static void initialize(void);
	void spawn(void);
	void hunt(void);
	void live(void);
	Event* hunt_event;
	String species;
public:
	Chameleon(void);
	~Chameleon(void);
	Color my_color(void) const;   // defines LifeForm::my_color
	static LifeForm* create(void);
	std::string species_name(void) const;
	Action encounter(const ObjInfo&);
	friend class Initializer<Chameleon>;
};


#endif /* !(_Craig_h) */

