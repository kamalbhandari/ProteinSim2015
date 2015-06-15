#include "integrator.h"

namespace integrators
{

	brownianIntegrator::brownianIntegrator(int nPart, double tempInit, double m, double dragCoeff, double dTime) :
	memX(new double[nPart]), memY(new double[nPart]), memZ(new double[nPart]),
	memCorrX(new double[nPart]), memCorrY(new double[nPart]), memCorrZ(new double[nPart])
	{

		//Stores the system information.
		memSize = nPart;
		temp = tempInit;
		mass = m;

		//Create vital variables
		gamma = dragCoeff;
		dt = dTime;
		y = gamma*dt;

		//Create G+B Variables
		coEff0 = exp(-y);
		coEff1 = (1.0-coEff0)/y;
		coEff2 = ((0.5*y*(1.0+coEff0))-(1.0-coEff0))/(y*y);
		coEff3 = (y-(1.0-coEff0))/(y*y);

		//Create G+B EQ 2.12 for gaussian width.
		double sig0 = temp/(mass*gamma*gamma);
		sig1 = std::sqrt( sig0 * getWidth(y) );
		sig2 = std::sqrt( -sig0 * getWidth(-y) );

		double gn = exp(y) - exp(-y) - (2.0*y);
		corr = (temp/(gamma*gamma)) * (gn/(sig1*sig2));
		dev = sqrt(1.0 - (corr*corr));

		//Creates the random device.
		std::random_device rd;
		gen = new std::mt19937(rd());
		Dist = new std::normal_distribution<double>(0.0,1.0);

		std::cout << "---Brownian integrator successfully added.\n\n";

	}

	brownianIntegrator::~brownianIntegrator()
	{
		delete &mass;
		delete &temp;
		delete &memSize;

		delete &gamma;
		delete &dt;
		delete &y;

		delete &coEff0;
		delete &coEff1;
		delete &coEff2;
		delete &coEff3;

		delete[] memX;
		delete[] memY;
		delete[] memZ;

		delete[] memCorrX;
		delete[] memCorrY;
		delete[] memCorrZ;

		delete &sig1;
		delete &sig2;
		delete &corr;
		delete &dev;

		delete gen;
		delete Dist;
	}

	double brownianIntegrator::getWidth(double gdt)
	{
		return (2*gdt) - 3.0 + (4.0*exp(-gdt)) - exp(-2.0*gdt);
	}

	int brownianIntegrator::nextSystem(double time, double dt, int nParticles, int boxSize, int cellScale, simulation::particle** items, physics::forces* f)
	{
		//Updates the force.
		for (int i=0; i < nParticles; i++)
		{
			f->getAcceleration(nParticles, boxSize, cellScale, time, items);
		}

		//Checks what method is needed.
		if (time == 0)
		{
			firstStep(time, dt, nParticles, boxSize, cellScale, items, f);
		}
		else
		{
			normalStep(time, dt, nParticles, boxSize, cellScale, items, f);
		}
		return 0;
	}

	int brownianIntegrator::firstStep(double time, double dt, int nParticles, int boxSize, int cellScale, simulation::particle** items, physics::forces* f)
	{

		for (int i=0; i < nParticles; i++)
		{

			//SEE GUNSTEREN AND BERENDSEN 1981 EQ 2.26

			memCorrX[i] = 0.0;
			memCorrY[i] = 0.0;
			memCorrZ[i] = 0.0;

			memX[i] = (*Dist)(*gen);
			memY[i] = (*Dist)(*gen);
			memZ[i] = (*Dist)(*gen);

			double m = 1.0/items[i]->getMass();
			double xNew = items[i]->getX() + (items[i]->getVX() * coEff1 * dt) + (items[i]->getFX() * coEff3 * dt * dt * m) + (sig1 * memX[i]);
			double yNew = items[i]->getY() + (items[i]->getVY() * coEff1 * dt) + (items[i]->getFY() * coEff3 * dt * dt * m) + (sig1 * memY[i]);
			double zNew = items[i]->getZ() + (items[i]->getVZ() * coEff1 * dt) + (items[i]->getFZ() * coEff3 * dt * dt * m) + (sig1 * memZ[i]);
			items[i]->setPos(xNew,yNew,zNew,boxSize);

		}

		return 0;
	}

	int brownianIntegrator::normalStep(double time, double dt, int nParticles, int boxSize, int cellScale, simulation::particle** items, physics::forces* f)
	{

		for (int i=0; i < nParticles; i++)
		{

			//SEE GUNSTEREN AND BERENDSEN 1981 EQ 2.26

			//New random walk.
			memCorrX[i] = (*Dist)(*gen);
			memCorrY[i] = (*Dist)(*gen);
			memCorrZ[i] = (*Dist)(*gen);

			//Correlation to last random walk.
			memCorrX[i] = sig2 * ((corr * memX[i])+(dev * memCorrX[i]));
			memCorrY[i] = sig2 * ((corr * memY[i])+(dev * memCorrY[i]));
			memCorrZ[i] = sig2 * ((corr * memZ[i])+(dev * memCorrZ[i]));

			memX[i] = (*Dist)(*gen);
			memY[i] = (*Dist)(*gen);
			memZ[i] = (*Dist)(*gen);

			double m = 1.0/items[i]->getMass();

			double xNew = ((1.0+coEff0) * items[i]->getX())
				- (coEff0 * items[i]->getX0())
				+ (m * dt * dt * coEff1 * items[i]->getFX()) 
				+ (m * dt * dt * coEff2 * (items[i]->getFX() - items[i]->getFX0()))
				+ (sig1 * memX[i]) + (coEff0 * memCorrX[i]);

			double yNew = ((1.0+coEff0) * items[i]->getY()) 
				- (coEff0 * items[i]->getY0())
				+ (m * dt * dt * coEff1 * coEff1 * items[i]->getFY())
				+ (m * dt * dt * coEff2 * (items[i]->getFY() - items[i]->getFY0()))
				+ (sig1 * memY[i]) + (coEff0 * memCorrY[i]);

			double zNew = ((1.0+coEff0) * items[i]->getZ())
				- (coEff0 * items[i]->getZ0())
				+ (m * dt * dt * coEff1 * coEff1 * items[i]->getFZ())
				+ (m * dt * dt * coEff2 * (items[i]->getFZ() - items[i]->getFZ0()))
				+ (sig1 * memZ[i]) + (coEff0 * memCorrZ[i]);

			items[i]->setPos(xNew, yNew, zNew, boxSize);

		}

		return 0;
	}

}