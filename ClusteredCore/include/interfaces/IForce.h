#ifndef IFORCE_H_
#define IFORCE_H_

namespace PSim {

/**
 * @class IForce
 * @author Sawyer Hopkins
 * @date 06/27/15
 * @file force.h
 * @brief Contract for forces.
 */
class IForce {

protected:

	std::string name;

public:

	//Header Version.
	static const int version = 1;

	virtual ~IForce() {};

	/**
	 * @brief Virtual methods for forces of various parameters.
	 * @param index The index particle to find the force on.
	 * @param nPart The number of particles in the system.
	 * @param boxSize The size of the system.
	 * @param time The current system time.
	 * @param itemCell The cell containing the index particle.
	 * @param items All particles in the system.
	 */
	virtual void getAcceleration(int index, double* sortedParticles, double* particleForce, vector<tuple<int,int>>* particleHashIndex, vector<tuple<int,int>>* cellStartEnd, systemState* state)=0;

	/**
	 * @brief Flag for a force dependent time.
	 * @return True for time dependent. False otherwise.
	 */
	virtual bool isTimeDependent()=0;

	/**
	 * Called after integration. Allows for the back-injection of the system state into the force.
	 * Useful for creating programmatic changes to the force depending on system state.
	 * @param state
	 */
	virtual void quench(systemState* state) {};

	/**
	 * @brief Get the name of the force for logging purposes.
	 * @return
	 */
	std::string getName() {
		return name;
	}

/** For nonlocal calculations that require a two-step force calculation build with -DWITHPOST */
#ifdef WITHPOST
	virtual void postRoutine(int index, double* sortedParticles, double* particleForce, vector<tuple<int,int>>* particleHashIndex, vector<tuple<int,int>>* cellStartEnd, systemState* state)=0;
#endif

};

typedef IForce* create_Force(config*);

}

#endif /* IFORCE_H_ */

