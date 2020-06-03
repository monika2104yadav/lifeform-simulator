#if !(_Human_h)
#define _Human_h 1

#include "LifeForm.h"
#include "Init.h"
#include "String.h"

class Human :public LifeForm
{
private:
	double perceive_radius;
	double quad_set_course_speed(vector<ObjInfo>& vector);
	bool check_same_specie(const ObjInfo&);
	void encode_specie_name();
	void decode_specie_name(String input, double& time, double& health);
	String species;
protected:
	static void initialize(void);
	double measure_spontaneity(ObjList&);
	void choose_course();
	void give_birth();
	void live();

	Event* choose_course_event;
public:
	Human(void);
	~Human(void);

	virtual std::string species_name(void) const;
	virtual std::string player_name(void);

	Color my_color(void) const;
	static LifeForm* create(void);

	Action encounter(const ObjInfo&);
	friend class Initializer<Human>;
};
#endif	
