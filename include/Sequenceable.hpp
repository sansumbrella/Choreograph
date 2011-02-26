/*
 *  Sequenceable.h
 *  BasicTween
 *
 *  Created by David Wicks on 2/14/11.
 *  Copyright 2011 David Wicks. All rights reserved.
 *
 */

#pragma once

namespace cinder
{
	namespace tween
	{
		//! Base interface for anything that can go on a Sequence
		class Sequenceable
		{
		public:
			virtual ~Sequenceable(){};
			//! advance time a specified amount
			virtual void step( double timestep ){};
			//! go to a specific time
			virtual void stepTo( double time ) = 0;
			
			//! is the animation finished?
			virtual bool isComplete(){ return false; }
			
			//! returns the duration of the sequenceable item
			virtual double getDuration(){ return 0; }
		};
	}
}

