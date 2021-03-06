/*The MIT License (MIT)

 Copyright (c) [2015] [Sawyer Hopkins]

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.*/

#include "analysisManager.h"

namespace PSim {
void analysisManager::postAnalysis(std::queue<std::string>* tests, particle** particles, systemState* state) {
	chatterBox.consoleMessage("Building cluster table.");
	std::vector<std::vector<particle*>> clusterPool = findClusters(particles, state->nParticles);
	chatterBox.consoleMessage("Loaded " + tos(clusterPool.size()) + " clusters.");
	while (tests->size() > 0) {
		std::string soda = PSim::util::tryPop(tests);

		if ((soda == "--coorhist") || (soda == "-CH")) {
			PSim::util::writeTerminal(
					"\nRunning structural histrogram analysis.\n",
					PSim::Colour::Green);
			coordinationHistogram(particles, state->nParticles);
		}
		if ((soda == "--clusthist") || (soda == "-CLH")) {
			PSim::util::writeTerminal(
					"\nRunning cluster size histrogram analysis.\n",
					PSim::Colour::Green);
			clusterSizeHistogram(clusterPool);
		}
		if ((soda == "--clustcoor") || (soda == "-CLC")) {
			PSim::util::writeTerminal(
					"\nRunning cluster structural histrogram analysis.\n",
					PSim::Colour::Green);
			clusterCoorHistogram(clusterPool);
		}
	}
}
}

