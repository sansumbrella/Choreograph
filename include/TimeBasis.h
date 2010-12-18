/*
 *  TimeBasis.h
 *  Clearing
 *
 *  Created by David Wicks on 6/21/10.
 *  Copyright 2010 David Wicks. All rights reserved.
 *	
 *	TimeBasis functions return a normalized time
 *	Given the start of a cycle and the duration of one cycle,
 *	They return a value in the range [0,1.0]
 *	
 *	They enable looping, ping-ponging, and reversing of tweens
 */

#pragma once

#include "cinder/app/App.h"
#include "cinder/CinderMath.h"

namespace cinder {
	namespace tween {
		struct TimeBasis {
			static double linear( double start, double duration )
			{	
				return math<double>::min( ( app::getElapsedSeconds() - start ) / duration, 1.0 );
			}
			
			static double linearByFrame( double start, double duration )
			{
				return math<double>::min( (double)( app::getElapsedFrames() - start ) / duration, 1.0 );
			}
			
			static double pingpong( double start, double duration )
			{
				double t2 = fmod( (app::getElapsedSeconds() - start), (duration*2) );
				if( t2 > duration ){
					return ((duration*2) - t2)/duration;
				} else {
					return t2/duration;
				}
			}
			
			static double repeat( double start, double duration )
			{
				double t = fmod( (app::getElapsedSeconds() - start), duration );
				return t / duration;
			}
			
			static double reverse( double start, double duration )
			{
				return math<double>::max( 1.0 - ( ( app::getElapsedSeconds() - start ) / duration ), 0.0 );
			}
		};
	}
}
