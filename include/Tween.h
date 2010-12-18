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

#include "cinder/app/App.h"
#include "cinder/Cinder.h"
#include "Easing.h"
#include "TimeBasis.h"

namespace cinder {
	namespace tween {
		
		//Non-templated base class to allow us to have a list containing all types of Tween
		class Tweenable {
		public:
			virtual ~Tweenable(){};
			virtual void update(){};
			virtual bool isComplete(){return true;}
			
			virtual void reverse(){};
			virtual void loop(){};
			virtual void pingpong(){};
			virtual void useFrames(){};
			virtual void jumpToFrame( int frame ){};
			virtual void delay( float amt ){};
			
			//
			void setDuration(double duration){ mDuration=duration; }
			
			// todo
			virtual void pause(){};
			
		protected:
			double mDuration;
		};
		
		typedef std::shared_ptr<Tweenable> TweenRef;
		
		//Our templated tween design
		template<typename T>
		class Tween : public Tweenable{
		public:
			// build a tween with a target, target value, duration, and optional ease function
			Tween<T>(T* target, T targetValue, double duration, double (*easeFunction)(double t)=Quadratic::easeInOut, double (*timeFunction)(double s, double d)=TimeBasis::linear )
			{
				mTarget = target;
				mStartValue = *target;
				mTargetValue = targetValue;
				mChange = mTargetValue - mStartValue;
				
				mStartTime = app::getElapsedSeconds();
				mDuration = duration;
				mT = 0.0;
				mComplete = false;
				
				mEaseFunction = easeFunction;
				mTimeFunction = timeFunction;
			}
			
			Tween<T>(T* target, T startValue, T targetValue, double duration, double (*easeFunction)(double t)=Quadratic::easeInOut, double (*timeFunction)(double s, double d)=TimeBasis::linear )
			{
				mTarget = target;
				mStartValue = startValue;
				mTargetValue = targetValue;
				mChange = mTargetValue - mStartValue;
				
				mStartTime = app::getElapsedSeconds();
				mDuration = duration;
				mT = 0.0;
				mComplete = false;
				
				mEaseFunction = easeFunction;
				mTimeFunction = timeFunction;
			}
			
			~Tween<T>(){}
			
			// this could be modified in the future to allow for a PathTween
			virtual void update()
			{
				mT = mTimeFunction( mStartTime, mDuration );
				updateTarget();	
			}
			
			virtual void updateTarget()
			{
				if( mT < 1.0 && mT > 0.0 )
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
			
			void useFrames()
			{
				// not quite as flexible as I'd like
				// will need to reconsider the use of timebasis functions
				setTimeFunction(TimeBasis::linearByFrame);
				mStartTime = app::getElapsedFrames();
			}
			
			void delay(float amt)
			{
				mStartTime += amt;
			}
			
			void jumpToFrame(int frame)
			{
				mT = (frame-mStartTime) / mDuration;
				updateTarget();
			}
			
			void reverse()
			{
				mStartTime += mDuration - ( app::getElapsedSeconds() - mStartTime );
				setTimeFunction(TimeBasis::reverse);
			}
			
			T* getTarget(){ return mTarget; }
			double getStartTime(){ return mStartTime; }
			bool isComplete(){ return mComplete; }
			
			void setEaseFunction( double (*easeFunction)(double t) ) { mEaseFunction = easeFunction; }
			void setTimeFunction( double (*timeFunction)(double start, double duration) ){ mTimeFunction = timeFunction; }
			
			static bool remove( Tween* tween ) { delete tween; return true; }
			
		private:			
			T* mTarget;
			T mStartValue, mChange, mTargetValue;
			
			double mT;	// normalized time
			double mStartTime;
			bool mComplete;
			
			// how we move between points in time
			double (*mEaseFunction)(double t);
			// how we interpret time
			// might need to be a static object, so it can also handle completion
			double (*mTimeFunction)(double start, double duration);
		};
		
	} //tween
} //cinder