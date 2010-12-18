/*
 *  Easing.h
 *  BasicTween
 *
 *  Created by David Wicks on 5/27/10.
 *  Copyright 2010 David Wicks. All rights reserved.
 *
 *  Easing functions Derived from Robert Penner's work
 *  http://www.robertpenner.com/easing/
 *
 *  Ease functions take a start value, the final change in value, and the current normalized timestep
 *	They return the value of the animation curve at the provided point in time
 *	
 *	For each easing function fn,
 *	fn( 0.0 ) will return 0.0, and fn( 1.0 ) will return 1.0
 *	in between, they may return any value, including those outside [0,1.0]
 * 
 */

#pragma once

namespace cinder {

	namespace tween {
		
		struct Quadratic {
			static double easeIn( double t ){
				//c*(t/=d)*t + b;
				return t * t;
			}
			static double easeOut( double t ){ 
				//-c *(t/=d)*(t-2) + b;
				return - t * (t-2);
			}
			static double easeInOut( double t ){
				//if ((t/=d*0.5) < 1) return c*0.5*t*t + b;
				//return -c*0.5 * ((--t)*(t-2) - 1) + b;
				t *= 2;
				if (t < 1) return 0.5 * t * t;
				
				t -= 1;
				return -0.5 * ((t)*(t-2) -1);
			}
		};
		
		
		struct Linear {
			static double noEase( double t ) {
				return t;
			}
		};
		
		struct Back {
			const static float ks = 1.70158;
			
			static double easeIn( double t ){
				return t * t * ((ks+1)*t - ks);
			}
			static double easeOut( double t ){ 
				t -= 1;
				return (t*t*((ks+1)*t + ks) + 1);
			}
			static double easeInOut( double t ){
				t *= 2;
				float s = ks * 1.525;
				
				if (t < 1) return 0.5*(t*t*(((s)+1)*t - s));
				return 0.5*((t-=2)*t*(((s)+1)*t + s) + 2);
				
			}
		};
		
	}
}