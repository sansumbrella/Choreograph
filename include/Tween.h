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
#include "cinder/CinderMath.h"
#include "Easing.h"
#include "Sequenceable.hpp"

namespace cinder {
	namespace tween {
		
		//Non-templated base class to allow us to have a list containing all types of Tween
		class Tweenable : public Sequenceable {
		public:
			Tweenable( void *data ) : mTargetVoid( data ) {}
			virtual ~Tweenable(){};
			
			//! change how the tween moves through time
			void setEaseFunction( double (*easeFunction)(double t) ) { mEaseFunction = easeFunction; }
			
			//! change how the tween thinks about time
			void setTimeFunction( double (*timeFunction)(double start, double duration) ){ mTimeFunction = timeFunction; }
			
			const void	*getTargetVoid() const { return mTargetVoid; }
			//! change the duration of the tween
			void setDuration( double duration ){ mDuration = duration; }
			//! returns the duration of the sequenceable item
			virtual double getDuration(){ return 0; }
			
		protected:
			double	mDuration;
			void	*mTargetVoid;
			// how we interpret time
			double (*mTimeFunction)(double start, double duration);
			// how we move between points in time
			double (*mEaseFunction)(double t);
		};
		
		typedef std::shared_ptr<Tweenable> TweenRef;
		
		//Our templated tween design
		template<typename T>
		class Tween : public Tweenable{
		public:
			// build a tween with a target, target value, duration, and optional ease function
			Tween<T>( T *target, T targetValue, double startTime, double duration, double (*easeFunction)(double t)=Quadratic::easeInOut )
				: Tweenable( target )
			{
				mTarget = target;
				mStartValue = *target;
				mEndValue = targetValue;
				mValueDelta = mEndValue - mStartValue;
				
				mStartTime = startTime;
				mDuration = duration;
				mT = 0.0;
				mComplete = false;
				
				mEaseFunction = easeFunction;
			}
			
			Tween<T>( T *target, T startValue, T targetValue, double startTime, double duration, double (*easeFunction)(double t)=Quadratic::easeInOut )
				: Tweenable( target )
			{
				mTarget = target;
				mStartValue = startValue;
				mEndValue = targetValue;
				mValueDelta = mEndValue - mStartValue;
				
				mStartTime = startTime;
				mDuration = duration;
				mT = 0.0;
				mComplete = false;
				
				mEaseFunction = easeFunction;
			}
			
			~Tween<T>(){}
			
			virtual void stepTo( double newTime )
			{
				mT = math<double>::min( (newTime - mStartTime) / mDuration, 1 );
				if( newTime < mStartTime + mDuration ){ mComplete = false; }
				
				updateTarget();	
			}
			
			// this could be modified in the future to allow for a PathTween
			virtual void updateTarget()
			{
				if( ! mComplete )
				{
					if( mT > 0.0 && mT < 1.0 )
					{
						*mTarget = mStartValue + mValueDelta * mEaseFunction( mT );
					} else if ( mT == 1.0 )
					{	// at the completion point, set to target value
						*mTarget = mEndValue;
						mComplete = true;
					} else
					{
						
					}
				}
			}
			
			T* getTarget(){ return mTarget; }
			double getStartTime(){ return mStartTime; }
			bool isComplete(){ return mComplete; }
			
		private:			
			T* mTarget;
			T mStartValue, mValueDelta, mEndValue;
			
			double mT;	// normalized time
			bool mComplete;
		};
	} //tween
} //cinder