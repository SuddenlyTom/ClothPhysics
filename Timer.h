


#ifndef TIMER_H
#define TIMER_H
class Timer
{
public:
	Timer();

	float getGameTime()const;  // in seconds
	float getDeltaTime()const; // in seconds

	void reset(); // Call before message loop.
	void start(); // Call when unpaused.
	void stop();  // Call when paused.
	void tick();  // Call every frame.

private:
	double mSecondsPerCount;
	double mDeltaTime;

	__int64 mBaseTime;
	__int64 mPausedTime;
	__int64 mStopTime;
	__int64 mPreviousTime;
	__int64 mCurrentTime;

	bool mStopped;
};

#endif