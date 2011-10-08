#ifndef __EVENT_TIMER_H__
#define __EVENT_TIMER_H__

#include <windows.h>

namespace Utilities {
	namespace Common{
		struct ProfTimer {
			void Start(void) {
				DWORD_PTR threadAffMask = SetThreadAffinityMask(GetCurrentThread(),1);

				QueryPerformanceFrequency(&ticksPerSecond);
				QueryPerformanceCounter(&tick);
				
				SetThreadAffinityMask(GetCurrentThread(),threadAffMask);
			};
			void Stop(void) {
				DWORD_PTR threadAffMask = SetThreadAffinityMask(GetCurrentThread(),1);
				QueryPerformanceCounter(&tock);
				SetThreadAffinityMask(GetCurrentThread(),threadAffMask);
			};
			double GetDurationInSecs(void)
			{
				double duration = (double)(tock.QuadPart-tick.QuadPart)/(double)ticksPerSecond.QuadPart;
				return duration;
			}
  			LARGE_INTEGER ticksPerSecond;
			LARGE_INTEGER tick;   // A point in time
			LARGE_INTEGER tock;
			LARGE_INTEGER time;   // For converting tick into real time
		};

		class EventTimer : public ProfTimer {
		public:
			EventTimer(){

				sprintf(name,"Default");
				Init();
			};
			EventTimer(char *pName){
				sprintf(name,"%s",pName);
				Init();
			};
			~EventTimer(){
			
			}
			void Init(){
				_nCalled = 0;
				_dDuration = 0;
				_dBefore = 0;
				_dAfter = 0;
				_isTiming = false;
				_isPaused = false;
				_isEnabled = true;

				Start();
			}
			void Disable(){ _isEnabled = false; }
			void Enable() { _isEnabled = true;  }
			void BeforeEvent(){
				if(!_isEnabled)return;
				if(_isTiming)printf("Warning:  EventTimer[%s] before event called twice without a call to After Event.\n",name);
				
				if(_isPaused){
					printf("Warning:  BeforeEvent[%s] called before UnPause\n",name);
					UnPause();
				}
				_isTiming = true;
				Stop();
				_dBefore = GetDurationInSecs();
				_dPauseDuration = 0;
			}
			void AfterEvent(){
				if(!_isEnabled)return;
				if(!_isTiming){
					printf("EventTimer[%s] needs to call BeforeEvent() first.  Ignoring timing\n",name);
					return;
				}
				if(_isPaused){
					printf("Warning:  AfterEvent[%s] called before unpause.\n",name);
					UnPause();
				}
				_isTiming = false;
				Stop();
				_dAfter = GetDurationInSecs();
				_nCalled++;
				//printf("%f %f\n",_dAfter,_dBefore);
				_dDuration+=((_dAfter-_dBefore)- _dPauseDuration);
			}
			double GetTimeSinceLastEvent() const { return _dAfter - _dBefore; }
			double GetDuration() const { return _dDuration; }
			void Pause(){
				if(!_isEnabled)return;
				if(_isPaused)printf("Warning:  EventTimer[%s] Pause() called twice in a row\n",name);

				Stop();
				_dPauseBefore = GetDurationInSecs();
				_isPaused = true;
			}
			void UnPause(){
				if(!_isEnabled)return;
				Stop();
				_dPauseAfter = GetDurationInSecs();

				_dPauseDuration+=(_dPauseAfter - _dPauseBefore);
				_isPaused = false;
			}
			void Display(){
				if(!_isEnabled)return;
				printf(" -- EventTimer: %s ----\n",name);
				printf("   - Number of Calls: %d\n",_nCalled);
				printf("   - Total Time: %f\n",_dDuration);
				printf("   - Timer per event: %f\n",_dDuration / _nCalled);
				printf(" ----------------------\n\n");
			}
			void SetName(char *aName){
				sprintf(name,"%s",aName);
			}
			bool isTiming() const { return _isTiming;}
			int GetNumCalled() const { return _nCalled;}
			double GetTotalTime() const { return _dDuration;}
			double GetTimePerEvent() const { return _dDuration / _nCalled; }
			
		private:
			char name[256];
			bool _isTiming;
			bool _isPaused;
			bool _isEnabled;
			int _nCalled;

			double _dPauseBefore, _dPauseAfter;
			double _dPauseDuration;

			double _dBefore,_dAfter;
			double _dDuration;
		};
	};
};

#endif //__EVENT_TIMER_H__