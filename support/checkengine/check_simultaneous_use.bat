
rem this illustrates that if you connect to the same matlab /Automation engine twice, from different processes, you get crossover (here, they both use the same RNG by calling rand() interleaved)
start checkengine workout1 thread
start checkengine workout2 thread
