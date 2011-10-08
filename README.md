EventTimer
==========

EventTimer is a simple high resolution event timing class.  It is a subclass of ProfTimer,
a simple timing class.

Usage
-----

    #include <EventTimer/EventTimer.h>
    using namespace Utilities::Common;

    void main(){
        EventTimer timer("Dummy");
	
        timer.BeforeEvent();
	
	/** Do some stuff **//
	
        timer.AfterEvent();
	
        printf("Elapsed time between events: %f seconds\n",timer.GetTimeSinceLastEvent());
        printf("Tital Elapsed time, exluding pauses: %f seconds\n",timer.GetDuration());
    }
