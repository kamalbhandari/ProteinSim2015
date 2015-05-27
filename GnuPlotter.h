#ifndef GNUPLOTTER_H
#define GNUPLOTTER_H

#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

namespace Plotting
{

class GnuPlotter
{
	public:

		//Sends the system to GNUPlot.
		static void plot(int size, float* x, float* y, float* z);
		//Writes the system as CSV.
		static void writeFile(int size, float* x, float* y, float* z, std::string name);

};

}

#endif // GNUPLOTTER_H
