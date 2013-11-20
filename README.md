EventTimer
==========

EventTimer is a simple high resolution event timing class.  It is a subclass of ProfTimer,
a simple timing class.

Usage (example requires c++11)
------------------------------

	#include "EventTimer/EventTimer.h"

	#include <chrono>
	#include <thread>

	int main(){

		{	// Simple timer, 1.65 seconds
			utilities::ProfTimer p;
			p.Start();
			std::this_thread::sleep_for(std::chrono::milliseconds(1565));
			p.Stop();	//total duration = 1.65 seconds
			
			printf("Duration: %f ms\n",p.GetDuration());
		}
		
		{ 	// Event timer, 1.25 seconds, 0.3 second pause (not included in total duration)
			utilities::EventTimer e("timer1");
			e.Before();
			std::this_thread::sleep_for(std::chrono::milliseconds(1250));
			e.Pause();
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
			e.Unpause();
			e.After();
			e.Dump();	//total duration = 1.25 seconds
		}
		
		{	// Event timer, 1.0 seconds, 500 iterations (2 ms per iteration)
			utilities::EventTimer e2("multitimer");
			e2.Before();
			for(int i = 0; i < 500; i++){
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
				e2.After();
			}
			e2.Dump();	//total duration = 1 second
		}
		return 0;
	}
