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
#include "cinder/app/App.h"
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
			void jumpToTime( double time );
			//! TODO: go to a specific part of the timeline
			void jumpToPercentCompletion( double percent ){};
			
			// I need a strategy for comparing tweens that works across types (perhaps an uid for each tween)
			//void removeTween( Tweenable* tween );
			void cancelAllTweens();
			void cleanup();
		private:
			TweenManager();
			std::vector< TweenRef > mTweens;
		
		};
	}

}