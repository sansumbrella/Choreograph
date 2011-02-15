/*
 *  TweenManager.h
 *  BasicTween
 *
 *  Created by David Wicks on 5/27/10.
 *  Copyright 2010 David Wicks. All rights reserved.
 *
 *	Runs all Tween objects provided to it
 *	Presents factory methods for creating Tweens (the recommended approach)
 *
 */

#pragma once
#include "Tween.h"
#include "cinder/Cinder.h"
#include <vector>
#include "cinder/Color.h"
#include "cinder/Vector.h"

namespace cinder {
	namespace tween {
			
		class TweenManager {
		
		public:
			static TweenManager& instance();
			
			//! add a tween to be managed
			void addTween( TweenRef );
			//! advance time based on target fps
			void step();
			//! advance time a specified amount
			void step( double timestep );
			//! go to a specific time
			void stepTo( double time );
			
			//! create a new tween and add it to the list
			template<typename T>
			TweenRef add( T *target, T targetValue, double duration, double (*easeFunction)(double t)=Quadratic::easeInOut, double (*timeFunction)(double s, double d)=TimeBasis::linear ) {
				mTweens.push_back( TweenRef( new Tween<T>( target, targetValue, mCurrentTime, duration, easeFunction, timeFunction ) ) );
				return mTweens.back();
			}
			
			//! replace an existing tween
			template<typename T>
			TweenRef replace( T *target, T targetValue, double duration, double (*easeFunction)(double t)=Quadratic::easeInOut, double (*timeFunction)(double s, double d)=TimeBasis::linear ) {
				TweenRef existingTween = findTween( target );
				if( existingTween )
					removeTween( existingTween );
				mTweens.push_back( TweenRef( new Tween<T>( target, targetValue, mCurrentTime, duration, easeFunction, timeFunction ) ) );
				return mTweens.back();
			}
			
			TweenRef	findTween( void *target );
			void		removeTween( TweenRef tween );
			
			//! remove all tweens from the timeline
			void clearTimeline();
			//! remove completed tweens from the timeline
			void clearFinishedTweens();
		private:
			TweenManager();
			double					mCurrentTime;
			std::vector< TweenRef > mTweens;
		
		};
	}

}