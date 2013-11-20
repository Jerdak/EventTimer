#ifndef __EVENT_TIMER_H__
#define __EVENT_TIMER_H__

#include <cstdlib>
#include <cstdio>
#include <string>
#include <utility>

#if (defined(_WIN32) || defined(_WIN64)) && !defined(__MINGW32__)
	#include <windows.h>
#elif defined(__GNUC__)
	#include <sys/time.h> 
#endif  

namespace utilities {
	/**	High resolution timer
		
		*Note:  Old documentation on MSDN [1] suggests using SetThreadAffinityMask
		as bugs in BIOS could result in inconsistent timings on multicore systems.
		Anecdotally we've seen this behaviour in the lab resulting in several negative
		deltas across a variety of hardware. 

		To mimic SetThreadAffinity in a *Nix environment I use clock_gettime() instead
		of just gettimeofday().  gettimeofday() is subject to modifications of the
		*Nix "wall-clock".  Something like NTP can alter the clock giving the user
		negative time deltas.
		
		Links
		[1] - http://msdn.microsoft.com/en-us/library/windows/desktop/ee417693(v=vs.85).aspx
	*/
	class ProfTimer {
	public:
		ProfTimer(){}
		
		#if (defined(_WIN32) || defined(_WIN64)) && !defined(__MINGW32__)
			// Start timer
			void Start(void) {
				DWORD_PTR threadAffMask = SetThreadAffinityMask(GetCurrentThread(),1);

				QueryPerformanceFrequency(&ticks_per_second_);
				QueryPerformanceCounter(&tick_);
				SetThreadAffinityMask(GetCurrentThread(),threadAffMask);     //make sure to revert affinity
			};
			
			// Stop timer
			void Stop(void) {
				DWORD_PTR threadAffMask = SetThreadAffinityMask(GetCurrentThread(),1);
				
				QueryPerformanceCounter(&tock_);
				SetThreadAffinityMask(GetCurrentThread(),threadAffMask);	//make sure to revert affinity
			};
			
			// Get duration between last call to Start() and Stop(), respectively.
			virtual double GetDurationInSecs(void)
			{
				double duration = (double)(tock_.QuadPart-tick_.QuadPart)/(double)ticks_per_second_.QuadPart;
				return duration;
			}
		private:
			LARGE_INTEGER ticks_per_second_;
			LARGE_INTEGER tick_;   // A point in time
			LARGE_INTEGER tock_;
		#elif defined(__GNUC__)
		
			// Start timer
			void Start(){
				clock_gettime(CLOCK_MONOTONIC,&tick_);
			}
			
			// Stop timer
			void Stop() {
				clock_gettime(CLOCK_MONOTONIC,&tock_);
			}
			
			// Get duration between last call to Start() and Stop(), respectively.
			virtual double GetDuration(){
				double ret = (tock_.tv_sec - tick_.tv_sec) * 1000;	//seconds to milliseconds
				ret += (tock_.tv_nsec - tick_.tv_nsec) * 1.0e-6;    //nanoseconds to milliseconds
				return ret;
			}
		private:
			timespec tick_;
			timespec tock_;
		#else 
			// TODO:  Test other implementations
			#pragma message("Unsupported compiler")
		#endif
		
		public:
			virtual double GetDurationSeconds(){
				return GetDuration()*1000.0;
			}
	};
	/**	Event timer class
	
		Time multiple events with allowances for pausing.
	*/
	class EventTimer : public ProfTimer {
		enum EVSTATUS { EVS_STARTED=0x1,EVS_PAUSED=0x4, EVS_ENABLED=0x8};
	public:
		EventTimer():
			name_("noname")
		{Reset();}
		
		EventTimer(const std::string &name):
			name_(name)
		{Reset();}
		
		// Reset timer, should always be called before method 'Before()'
		void Reset(){
			status_ = EVS_ENABLED;
			pause_duration_ = 0.0;
			event_duration_ = 0.0;
			num_calls_ = 0;
			
			event_durations_ = std::make_pair (0,0);
			pause_durations_ = std::make_pair (0,0);
			Start();
		}
	
		bool Enabled(){return status_&EVS_ENABLED;}
		bool Started() {return status_&EVS_STARTED;}
		bool Paused() {return status_&EVS_PAUSED;}
		
		// Call once before event to be timed
		void Before(){
			if(!Enabled())return;
			if(Started()) {
				printf("warning, event restarted without being stopped.");
			}
			if(Paused()){
				printf("Warning:  BeforeEvent[%s] called before UnPause\n",name_.c_str());
				Unpause();	
			}
			Stop();
			event_durations_.first = ProfTimer::GetDuration();
			
			status_ = EVSTATUS(status_ | EVS_STARTED);
		}
		
		// Call any number of times after 'Before()' to time multiple events.
		void After(){
			if(!Enabled())return;
			if(!Started()) {
				printf("error, event not started");
				return;
			}
			if(Paused()){
				printf("Warning:  AfterEvent[%s] called before UnPause\n",name_.c_str());
				Unpause();	
			}
			num_calls_ += 1;
			
			Stop();
			event_durations_.second = ProfTimer::GetDuration();
			event_duration_ = (event_durations_.second - event_durations_.first) - pause_duration_;
			//status_ = EVSTATUS(status_ & ~EVS_STARTED);
		}
		
		// Unpause timer
		void Unpause(){
			status_ = EVSTATUS(status_ & ~EVS_PAUSED);
			Stop();
			
			pause_durations_.second = ProfTimer::GetDuration();
			pause_duration_ += (pause_durations_.second - pause_durations_.first);
		}
		
		// Pause timer, event duration is ignored while paused.
		void Pause(){
			status_ = EVSTATUS(status_ | EVS_PAUSED);
			Stop();
			
			pause_durations_.first = ProfTimer::GetDuration();
		}
		double GetDuration(){
			return event_duration_;
		}
		
		void Dump(){
			printf("Eventtimer[%s]\n",name_.c_str());
			printf("  - IsEnabled: %d\n",Enabled());
			printf("  - IsStarted: %d\n",Started());
			printf("  - IsPaused: %d\n",Paused());
			printf("  - Number of calls: %d\n",num_calls_);
			printf("  - Total Time: %f\n",event_duration_);
			printf("  - Time per event: %f\n",event_duration_/num_calls_);
			
		}
		
	private:
		int num_calls_;
		std::pair<double,double> event_durations_;
		std::pair<double,double> pause_durations_;
		
		double event_duration_;
		double pause_duration_;
		
		std::string name_;
		EVSTATUS status_;
	};
}

#endif //__EVENT_TIMER_H__