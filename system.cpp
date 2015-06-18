#include "system.h"

namespace simulation
{

	/*-----------------------------------------*/
	/*----------CONSTRUCTOR/DESTRUCTOR---------*/
	/*-----------------------------------------*/

	system::system(int nPart, double conc, int scale, double m, double r, double sysTemp, double sysDT, int rnd, integrators::I_integrator* sysInt, physics::forces* sysFcs)
	{

		//Set time information
		currentTime = 0;
		dTime = sysDT;

		//Set the random number generator seed.
		seed = rnd;

		//Sets the system temperature.
		temp = sysTemp;

		//Set the number of particles.
		nParticles = nPart;

		//Set the integration method.
		integrator = sysInt;

		//Set the internal forces.
		sysForces = sysFcs;

		//Create a box based on desired concentration.
		double vP = nPart*(4.0/3.0)*atan(1)*4*r*r*r;
		boxSize = (int) cbrt(vP / conc);

		//Calculates the number of cells needed.
		cellSize = boxSize / scale;
		boxSize = cellSize * scale;
		cellScale = scale;
		int numCells = pow(scale,3.0);

		//Sets the actual concentration.
		concentration = vP/pow(boxSize,3.0);

		std::cout << "---System concentration: " << concentration << "\n";

		//Create particles.
		initParticles(r,m);

		//Create cells.
		initCells(numCells, scale);
		std::cout << "---Created: " << numCells << " cells from scale: " <<  scale << "\n";

	}

	system::~system()
	{
		//Deletes the particles
		for (int i=0; i < nParticles; i++)
		{
			delete[] particles[i];
		}
		delete[] particles;

		//Delete the constants.
		delete &nParticles;
		delete &concentration;
		delete &boxSize;
		delete &cellSize;
		delete &temp;
		delete &currentTime;
		delete &dTime;
		delete &seed;
		delete[] integrator;
		delete[] sysForces;
	}

	/*-----------------------------------------*/
	/*---------------SYSTEM INIT---------------*/
	/*-----------------------------------------*/

	void system::initCells(int numCells, int scale)
	{

		//Create the cells.
		cells = new cell***[scale];
		for(int i=0; i < scale; i++)
		{
			cells[i] = new cell**[scale];
			for(int j=0; j < scale; j++)
			{
				cells[i][j] = new cell*[scale];
				for(int k=0; k < scale; k++)
				{
					cells[i][j][k] = new cell;
				}
			}
		}

		//Set the cell neighbors.
		for(int x=0; x < scale; x++)
		{
			for(int y=0; y < scale; y++)
			{
				for(int z=0; z < scale; z++)
				{
					int left = x-1;
					int right = x+1;
					int top = y-1;
					int bot = y+1;
					int front = z-1;
					int back = z+1;

					if (x == 0)
					{
						left = (scale-1);
					}
					if (x == (scale-1))
					{
						right = 0;
					}

					if (y == 0)
					{
						top = (scale-1);
					}
					if (y == (scale-1))
					{
						bot = 0;
					}

					if (z == 0)
					{
						front = (scale-1);
					}
					if (z == (scale-1))
					{
						back = 0;
					}

					cells[x][y][z]->left = cells[left][y][z];
					cells[x][y][z]->right = cells[right][y][z];
					cells[x][y][z]->top = cells[x][top][z];
					cells[x][y][z]->bot = cells[x][bot][z];
					cells[x][y][z]->front = cells[x][y][front];
					cells[x][y][z]->back = cells[x][y][back];

				}
			}
		}

		//Assign the particle to their starting cell.
		for(int i=0; i < nParticles; i++)
		{
			int cx = particles[i]->getX() / cellSize;
			int cy = particles[i]->getY() / cellSize;
			int cz = particles[i]->getZ() / cellSize;

			particles[i]->setCell(cx,cy,cz);

			cells[cx][cy][cz]->addMember(particles[i]);

		}

	}

	void system::initParticles(double r, double m)
	{

		particles = new particle*[nParticles];

		//If there is no inital seed create one.
		if (seed==0)
		{
			std::random_device rd;
			seed=rd();
		}
		//Setup random uniform distribution generator.
		std::mt19937 gen(seed);
		std::uniform_real_distribution<double> distribution(0.0,1.0);
		
		//Iterates through all points.
		for(int i = 0; i < nParticles; i++)
		{
			particles[i] = new particle(i);
			particles[i]->setX( distribution(gen) * boxSize , boxSize);
			particles[i]->setY( distribution(gen) * boxSize , boxSize);
			particles[i]->setZ( distribution(gen) * boxSize , boxSize);

			particles[i]->setRadius(r);
			particles[i]->setMass(m);

		}

		std::cout << "---Added " << nParticles << " particles. Checking for overlap.\n";

		//Checks the system for overlap.
		initCheck(&gen, &distribution);

		std::cout << "---Overlap resolved. Created Maxwell distribution.\n";

		//Set initial velocity.
		maxwellVelocityInit(&gen, &distribution);

		std::cout << "---Maxwell distribution created. Creating cell assignment.\n\n";

	}

	void system::initCheck(std::mt19937* gen, std::uniform_real_distribution<double>* distribution)
	{
		//Keeps track of how many resolutions we have attempted.
		int counter = 0;

		//Search each particle for overlap.
		for(int i = 0; i < nParticles; i++)
		{
			//Is the problem resolved?
			bool resolution = false;
			//If not loop.
			while (resolution == false)
			{
				//Assume resolution.
				resolution = true;
				for(int j = 0; j < nParticles; j++)
				{
					//Exclude self interation.
					if (i != j)
					{
						//Gets the distance between the two particles.
						double distX = utilities::util::pbcDist(particles[i]->getX(),particles[j]->getX(),boxSize);
						double distY = utilities::util::pbcDist(particles[i]->getY(),particles[j]->getY(),boxSize);
						double distZ = utilities::util::pbcDist(particles[i]->getZ(),particles[j]->getZ(),boxSize);

						double radius = std::sqrt((distX*distX)+(distY*distY)+(distZ*distZ));

						//Gets the sum of the particle radius.
						double r = particles[i]->getRadius() + particles[j]->getRadius();

						//If the particles are slightly closer than twice their radius resolve conflict.
						if (radius < 1.1*r)
						{
							//Update resolution counter.
							counter++;

							//Throw warnings if stuck in resolution loop.
							if (counter > 10*nParticles)
							{
								std::cout << "Could create initial system.\n";
								std::cout << "Try decreasing particle density\n.";
							}

							//Assume new system in not resolved.
							resolution = false;

							//Set new uniform random position.
							particles[i]->setX( (*distribution)(*gen) * boxSize , boxSize );
							particles[i]->setY( (*distribution)(*gen) * boxSize , boxSize );
							particles[i]->setZ( (*distribution)(*gen) * boxSize , boxSize );
						}
					}
				}
			}
		}
	}

	void system::maxwellVelocityInit(std::mt19937* gen, std::uniform_real_distribution<double>* distribution)
	{
		double r1,r2;
		double vsum,vsum2;
		double sigold,vsig,ratio;
		int i;
		
		for(i=0; i<nParticles; i++)
		{
			r1=(*distribution)(*gen);
			r2=(*distribution)(*gen);

			particles[i]->setVX(sqrt(-2.0 * log(r1) ) * cos(8.0*atan(1)*r2));
		}

		for(i=0; i<nParticles; i++)
		{
			r1=(*distribution)(*gen);
			r2=(*distribution)(*gen);
			particles[i]->setVY(sqrt(-2.0 * log(r1) ) * cos(8.0*atan(1)*r2));
		}

		for(i=0; i<nParticles; i++)
		{
			r1=(*distribution)(*gen);
			r2=(*distribution)(*gen);
			particles[i]->setVZ(sqrt(-2.0 * log(r1) ) * cos(8.0*atan(1)*r2));
		}
		
		//maxwell for vx//
		vsum=0;
		vsum2=0;
		
		for(i=0; i<nParticles; i++)
		{
			double vx = particles[i]->getVX();
			vsum=vsum+vx;
			vsum2=vsum2+(vx*vx);
		}
		vsum=vsum/nParticles;
		vsum2=vsum2/nParticles;
		sigold=sqrt(vsum2-(vsum*vsum));

		vsig= sqrt(temp) ;
		ratio=vsig/sigold;

		for(i=0; i<nParticles; i++)
		{
			particles[i]->setVX(ratio*(particles[i]->getVX()-vsum));
		}
	////////////////////

		//maxwell for vy//
		vsum=0;
		vsum2=0;
		
		for(i=0; i<nParticles; i++)
		{
			double vy = particles[i]->getVY();
			vsum=vsum+vy;
			vsum2=vsum2+(vy*vy);
		}
		vsum=vsum/nParticles;
		vsum2=vsum2/nParticles;
		sigold=sqrt(vsum2-(vsum*vsum));

		vsig= sqrt(temp) ;
		ratio=vsig/sigold;

		for(i=0; i<nParticles; i++)
		{
			particles[i]->setVY(ratio*(particles[i]->getVY()-vsum));
		}
	////////////////////

		//maxwell for vz//
		vsum=0;
		vsum2=0;
		
		for(i=0; i<nParticles; i++)
		{
			double vz = particles[i]->getVZ();
			vsum=vsum+vz;
			vsum2=vsum2+(vz*vz);
		}
		vsum=vsum/nParticles;
		vsum2=vsum2/nParticles;
		sigold=sqrt(vsum2-(vsum*vsum));

		vsig= sqrt(temp) ;
		ratio=vsig/sigold;

		for(i=0; i<nParticles; i++)
		{
			particles[i]->setVZ(ratio*(particles[i]->getVZ()-vsum));
		}
	}

	/*-----------------------------------------*/
	/*------------PARTICLE HANDLING------------*/
	/*-----------------------------------------*/

	void system::moveParticle(int index, double x, double y, double z)
	{

		//Update the position of the particle.
		particles[index]->setX(x,boxSize);
		particles[index]->setY(x,boxSize);
		particles[index]->setZ(x,boxSize);

		//New cell
		int cX = particles[index]->getX() / cellSize;
		int cY = particles[index]->getY() / cellSize;
		int cZ = particles[index]->getZ() / cellSize;

		//Old cell
		int cX0 = particles[index]->getCX();
		int cY0 = particles[index]->getCY();
		int cZ0 = particles[index]->getCZ();

		//If cell has changed
		if ((cX != cX0) || (cY != cY0) || (cZ != cZ0))
		{
			particles[index]->setCell(cX,cY,cZ);
		}

	}

	void system::updateCells()
	{

		for (int index=0; index < nParticles; index++)
		{

			//New cell
			int cX = particles[index]->getX() / cellSize;
			int cY = particles[index]->getY() / cellSize;
			int cZ = particles[index]->getZ() / cellSize;

			//Old cell
			int cX0 = particles[index]->getCX();
			int cY0 = particles[index]->getCY();
			int cZ0 = particles[index]->getCZ();

			//If cell has changed
			if ((cX != cX0) || (cY != cY0) || (cZ != cZ0))
			{
				cells[cX0][cY0][cZ0]->removeMember(particles[index]);
				cells[cX][cY][cZ]->addMember(particles[index]);
				particles[index]->setCell(cX,cY,cZ);
			}

		}

	}

	void system::run(double endTime)
	{
		int counter = 0;
		while (currentTime < endTime)
		{
			//utilities::util::loadBar(currentTime,endTime,counter);
			integrator->nextSystem(currentTime, dTime, nParticles, boxSize, cells, particles, sysForces);
			updateCells();
			currentTime += dTime;
			counter++;
			std::cout << "time: " << counter << "\n";
		}
	}

	/*-----------------------------------------*/
	/*--------------SYSTEM OUTPUT--------------*/
	/*-----------------------------------------*/

	void system::writeSystem(std::string name)
	{
		//Create a stream to the desired file.
		std::ofstream myFile;
		myFile.open(name + ".txt");
		//Write each point in the system as a line of csv formatted as: X,Y,Z
		for (int i = 0; i < nParticles; i++)
		{
			myFile << particles[i]->getX() << "," << particles[i]->getY() << "," << particles[i]->getZ() << "\n";
		}
		//Close the stream.
		myFile.close();
	}

}

