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
			
			//! advance time in the animation to the given point
			virtual void stepTo( double newTime ) = 0;
			//! is the animation finished?
			virtual bool isComplete(){return true;}
			
			//! change the way time is handled by the tween
			virtual void reverse(){};
			virtual void loop(){};
			virtual void pingpong(){};
			
			//! push back the tween's start time
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
				mDuration = duration;
				mT = 0.0;
				mComplete = false;
				
				mEaseFunction = easeFunction;
				mTimeFunction = timeFunction;
			}
			
			~Tween<T>(){}
			
			// this could be modified in the future to allow for a PathTween
			virtual void stepTo( double newTime )
			{
				mT = mTimeFunction( newTime - mStartTime, mDuration );
				updateTarget();	
			}
			
			virtual void updateTarget()
			{
				if( mT > 0.0 && mT < 1.0 )
				{
					*mTarget = mStartValue + mChange * mEaseFunction( mT );
				} else if ( mT == 1.0 )
				{	// at the completion point, set to target value
					*mTarget = mTargetValue;
					mComplete = true;
				} else
				{
					
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
			}
			
			void reverse()
			{
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
			double mStartTime;
			bool mComplete;
			
			// how we move between points in time
			double (*mEaseFunction)(double t);
			// how we interpret time
			// might need to be a static object, so it can also handle completion
			double (*mTimeFunction)(double start, double duration);
		};

		/*
		 * Debating whether to add some predefined tween types
		 *
		typedef Tween<float> Tweenf;
		typedef Tween<Vec2f> Tween2f; 
		//*/
	} //tween
} //cinder