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
	class Sequenceable
	{
	public:
		virtual ~Sequenceable(){};
		//! advance time a specified amount
		virtual void step( double timestep ){};
		//! go to a specific time
		virtual void stepTo( double time ) = 0;
	};
}
}

