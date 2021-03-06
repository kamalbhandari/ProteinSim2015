/********************************************//**
*------------------AO POTENTIAL------------------
************************************************/
 #include "forceManager.h"
 #include "utilities.h"

using namespace PSim;
using namespace std;

/**
 * @class AOPotential
 * @author Sawyer Hopkins
 * @date 06/27/15
 * @file force.h
 * @brief AO Potential.
 */
class AOPotential : public PSim::IForce
{

private:

		//Variables vital to the force.
		double wellDepth;
		double kT;
		double cutOff;
		double dt;

		//Secondary variables.
		double coEff1;
		double coEff2;

		//Potential variables.
		double a1;
		double a2;
		double a3;

	public:

		/**
		 * @brief Creates an new AO Potential.
		 * @param cfg The address of the configuration file reader.
		 */
		AOPotential(config* cfg);
		/**
		 * @brief Releases the force from memory.
		 */
		~AOPotential();

		/**
		 * @brief Get the force from the AO Potential.
		 * @param index The index particle to calculated the force on.
		 * @param nPart The number of particles in the system.
		 * @param boxSize The size of the system.
		 * @param time The current system time.
		 * @param itemCell The cell containing the index particle.
		 * @param items All particles in the system.
		 */
		void getAcceleration(int index, double* sortedParticles, double* particleForce, vector<tuple<int,int>>* particleHashIndex, vector<tuple<int,int>>* cellStartEnd, systemState* state);
		/**
		 * @brief Flag for a force dependent time.
		 * @return True for time dependent. False otherwise. 
		 */
		bool isTimeDependent() { return false; }
		/**
		 * @brief Checks for particle interation between the index particle and all particles in the provided cell.
		 * @param boxSize The size of the system.
		 * @param time The current system time.
		 * @param index The particle to find the force on.
		 * @param itemCell The cell to check for interactions in.
		 */
		type3<double> iterCells(int index, int hash, double* sortedParticles, vector<tuple<int,int>>* cellStartEnd, systemState* state);

		void quench(systemState* state) {};
};

//Class factories.
extern "C" PSim::IForce* getForce(config* cfg)
{
	return new AOPotential(cfg);
}
