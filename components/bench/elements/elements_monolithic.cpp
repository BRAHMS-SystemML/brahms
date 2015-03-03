
//	compile with g++ -O3 or equivalent

#include "timing.h"
#include "scalable.h"


//	parameters
PARS pars;

//	state
vector<STATE> states;

//	benchmarking
Timer					timer;
DOUBLE					t_init;
DOUBLE					t_run;



//	bench version
void cm_system(ofstream* file)
{
	//	one state per process
	states.resize(pars.P);

	//	initialise
	timer.reset();
	for (int p=0; p<pars.P; p++)
		states[p].init(pars, p);
	t_init = timer.elapsed();

	//	run
	vector<STATE_TYPE> buffer;
	buffer.resize(pars.P * 2); // double buffer

	timer.reset();
	if (file)
	{
		for (UINT32 n = 0; n < pars.N; n++)
		{
			//	double buffer
			STATE_TYPE* ibuf;
			STATE_TYPE* obuf;
			if (n % 2)
			{
				obuf = &buffer[pars.P];
				ibuf = &buffer[0];
			}
			else
			{
				obuf = &buffer[0];
				ibuf = &buffer[pars.P];
			}

			for (UINT32 p = pars.P; p != 0; p--)
			{
				//	get output of this process
				UINT32 po = p - 1;

				//	take input from this process
				UINT32 pi = po - 1;
				if (!po) pi = pars.P - 1;

				//	run job
				STATE_TYPE* src = states[po].step(ibuf[pi]);
				file->write((const char*) src, pars.E * sizeof(STATE_TYPE));
				obuf[po] = *src;
			}
		}
	}

	else
	{
		for (UINT32 n = 0; n < pars.N; n++)
		{
			//	double buffer
			STATE_TYPE* ibuf;
			STATE_TYPE* obuf;
			if (n % 2)
			{
				obuf = &buffer[pars.P];
				ibuf = &buffer[0];
			}
			else
			{
				obuf = &buffer[0];
				ibuf = &buffer[pars.P];
			}

			for (UINT32 p = pars.P; p != 0; p--)
			{
				//	get output of this process
				UINT32 po = p - 1;

				//	take input from this process
				UINT32 pi = po - 1;
				if (!po) pi = pars.P - 1;

				//	run job
				obuf[po] = *(states[po].step(ibuf[pi]));
			}

			//	output of process 0
			//cerr << obuf[0] << endl;
		}
	}
	t_run = timer.elapsed();

	//	output timings
	ofstream tfile("timing");
	tfile << t_init << " " << t_run;
	tfile.close();
}

int main(int argc, char* argv[])
{
	//	read pars
	if (argc != 5 && argc != 6)
	{
		cerr << argv[0] << " P E O N [logfilename]" << endl;
		return 1;
	}

	//	note that atoX has undefined behavior when the resulting
	//	values cannot be represented - this is unfortunate, because
	//	atoX looks much nicer than strtoX here!
	pars.P 				= (UINT32)	strtol(argv[1],  (char**)NULL, 10);
	pars.E 				= (UINT32)	strtol(argv[2],  (char**)NULL, 10);
	pars.O 				= (UINT32)	strtol(argv[3],  (char**)NULL, 10);
	pars.N				= (UINT32)	strtol(argv[4],  (char**)NULL, 10);

	bool bench = argc == 5;

	if (bench)
	{
		cm_system(NULL);
		return 0;
	}

	else
	{
		ofstream file(argv[5], ios::out | ios::binary);
		if (file)
		{
			cm_system(&file);
			file.close();
			return 0;
		}
		cerr << "cannot open file - aborted" << endl;
	}

	return 1;
}
