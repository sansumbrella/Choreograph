/*
 *  Tween.h
 *  BasicTween
 *
 *  Created by David Wicks on 5/26/10.
 *  Copyright 2010 David Wicks. All rights reserved.
 *
 *	Tweens any type that supports operators + * /
 *	Tween<type>( &object, toValue, overTime, withEasing );
 *
 */

#pragma once

#include "cinder/Cinder.h"
#include "Easing.h"
#include "TimeBasis.h"

namespace cinder {
	namespace tween {
		
		//Non-templated base class to allow us to have a list containing all types of Tween
		class Tweenable {
		public:
			Tweenable( void *data ) : mTargetVoid( data ) {}
			virtual ~Tweenable(){};
			
			//! advance time in the animation
			virtual void update( double newTime ) = 0;
			virtual void step( double timestep ) = 0;
			virtual void jumpToTime( double time ) = 0;
			//! is the animation finished?
			virtual bool isComplete(){return true;}
			
			// these will likely be moved to the timeline/manager for control
			virtual void reverse(){};
			virtual void loop(){};
			virtual void pingpong(){};
			
			virtual void delay( float amt ){};
			
			//! change the duration of the tween
			void setDuration(double duration){ mDuration=duration; }
			
			// todo
			virtual void pause(){};
			
			const void	*getTargetVoid() const { return mTargetVoid; }
			
		protected:
			double	mDuration;
			void	*mTargetVoid;
		};
		
		typedef std::shared_ptr<Tweenable> TweenRef;
		
		//Our templated tween design
		template<typename T>
		class Tween : public Tweenable{
		public:
			// build a tween with a target, target value, duration, and optional ease function
			Tween<T>( T *target, T targetValue, double startTime, double duration, double (*easeFunction)(double t)=Quadratic::easeInOut, double (*timeFunction)(double s, double d)=TimeBasis::linear )
				: Tweenable( target )
			{
				mTarget = target;
				mStartValue = *target;
				mTargetValue = targetValue;
				mChange = mTargetValue - mStartValue;
				
				mStartTime = startTime;
				mCurrentTime = mStartTime;
				mDuration = duration;
				mT = 0.0;
				mComplete = false;
				
				mEaseFunction = easeFunction;
				mTimeFunction = timeFunction;
			}
			
			Tween<T>( T *target, T startValue, T targetValue, double startTime, double duration, double (*easeFunction)(double t)=Quadratic::easeInOut, double (*timeFunction)(double s, double d)=TimeBasis::linear )
				: Tweenable( target )
			{
				mTarget = target;
				mStartValue = startValue;
				mTargetValue = targetValue;
				mChange = mTargetValue - mStartValue;
				
				mStartTime = startTime;
				mCurrentTime = mStartTime;
				
				mDuration = duration;
				mT = 0.0;
				mComplete = false;
				
				mEaseFunction = easeFunction;
				mTimeFunction = timeFunction;
			}
			
			~Tween<T>(){}
			
			// this could be modified in the future to allow for a PathTween
			virtual void update( double newTime )
			{
				mT = mTimeFunction( newTime - mStartTime, mDuration );
				updateTarget();	
			}
			
			//! advance our tween the specified amount of time
			//! this setup allows us to step backward, as well
			virtual void step( double timestep )
			{
				mCurrentTime += timestep;
				mT = lmap( mCurrentTime, mStartTime, mStartTime + mDuration, 0.0, 1.0 );
				updateTarget();
			}
			
			virtual void jumpToTime( double time )
			{
				mCurrentTime = mStartTime + time;
				mT = lmap( mCurrentTime, mStartTime, mStartTime + mDuration, 0.0, 1.0 );
				updateTarget();
			}
			
			virtual void updateTarget()
			{
				if( mT > 0.0 && mT < 1.0 )
				{
					*mTarget = mStartValue + mChange * mEaseFunction( mT );
				} else if ( mT >= 1.0 )
				{	// this check needs to be made less fragile
					*mTarget = mTargetValue;
					mComplete = true;
				} else
				{
					//*mTarget = mStartValue;
				}
			}
			
			void loop()
			{
				setTimeFunction(TimeBasis::repeat);
			}
			
			void pingpong()
			{
				setTimeFunction(TimeBasis::pingpong);
			}
			
			void delay(float amt)
			{
				mStartTime += amt;
				mCurrentTime = mStartTime;
			}
			
			void jumpToFrame(int frame)
			{
				mT = (frame-mStartTime) / mDuration;
				updateTarget();
			}
			
			void reverse()
			{
//				mStartTime += mDuration - ( app::getElapsedSeconds() - mStartTime );
				setTimeFunction(TimeBasis::reverse);
			}
			
			T* getTarget(){ return mTarget; }
			double getStartTime(){ return mStartTime; }
			bool isComplete(){ return mComplete; }
			
			void setEaseFunction( double (*easeFunction)(double t) ) { mEaseFunction = easeFunction; }
			void setTimeFunction( double (*timeFunction)(double start, double duration) ){ mTimeFunction = timeFunction; }
			
		private:			
			T* mTarget;
			T mStartValue, mChange, mTargetValue;
			
			double mT;	// normalized time
			double mCurrentTime; // time in seconds or frames
			double mStartTime;
			bool mComplete;
			
			// how we move between points in time
			double (*mEaseFunction)(double t);
			// how we interpret time
			// might need to be a static object, so it can also handle completion
			double (*mTimeFunction)(double start, double duration);
		};

		/*
		 * Debating whether to add some of these predefined tween types
		 *
		
		typedef Tween<float> Tweenf;
		 
		//*/
	} //tween
} //cinder