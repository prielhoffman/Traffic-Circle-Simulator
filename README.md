# Traffic Circle Simulator
This repository contains a multi-threaded simulation program for modeling traffic flow in a traffic circle. The program is designed to help the Ministry of Transportation (MOT) predict congestion at a junction, based on specific parameters such as car generation intervals, car movement intervals, and the probability of cars exiting the circle.

# Overview
The simulator models a traffic circle built from square units. Each unit can contain a maximum of one car at any given time. The simulation runs for a specified period, during which cars are generated at four corners (entry points) and move around the circle, potentially exiting at sink points located at each corner.

# Key Features
* Multi-threaded Simulation: The program utilizes multiple threads to manage car movements concurrently across different squares in the circle.
* Mutual Exclusion: Ensures that only one car can occupy a square at any given time using mutexes to avoid race conditions.
* Randomized Car Generation: Cars are generated at randomized intervals within a specified range.
* Snapshot of Circle State: 10 times during the simulation, the program takes a snapshot of the circle's state and prints it, showing where the cars are located.
* Car Exiting: Cars can exit the simulation at sink points with a certain probability after completing at least one side of the square.

# How It Works
* Car Generation: Cars are generated at each corner of the traffic circle at random intervals between MIN_INTER_ARRIVAL_IN_NS and MAX_INTER_ARRIVAL_IN_NS.
* Car Movement: Cars attempt to move to the next square every INTER_MOVES_IN_NS. If the next square is occupied, the car stays in its current position until the next attempt.
* Exiting the Circle: At each sink point (located at the corners of the circle), a car may exit the circle with a probability of FIN_PROB. If it doesn't exit, it continues moving around the circle.
* Snapshots: During the simulation, 10 snapshots are taken to display the state of the traffic circle, showing which squares are occupied by cars (*), which are empty ( ), and which are part of the traffic circle (@).

# Notes
The simulation may yield unexpected results if extreme values are chosen for the parameters (e.g., generating new cars too frequently may lead to resource exhaustion).
Ensure the operating environment supports pthread for multi-threading.
