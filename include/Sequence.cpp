/*
 *  Sequence.cpp
 *  BasicTween
 *
 *  Created by David Wicks on 5/27/10.
 *  Copyright 2010 David Wicks. All rights reserved.
 *
 */

#include "Sequence.h"
#include "cinder/app/App.h"

using namespace cinder;
using namespace cinder::tween;
typedef std::vector< TweenRef >::iterator t_iter;
typedef std::vector< CueRef >::iterator c_iter;


Sequence::Sequence()
{
	mCurrentTime = 0;
}

void Sequence::step()
{	// would like to use getAverageFps, but it doesn't work statically (yet)
	step( 1.0 / app::getFrameRate() );
}

void Sequence::step( double timestep )
{
	mCurrentTime += timestep;
	
	for( t_iter iter = mTweens.begin(); iter != mTweens.end(); ++iter )
	{
		(**iter).stepTo( mCurrentTime );
	}
	
	for( c_iter iter = mCues.begin(); iter != mCues.end(); ++iter )
	{
		(**iter).stepTo( mCurrentTime );
	}
}

void Sequence::stepTo( double time )
{	
	mCurrentTime = time;
	
	for( t_iter iter = mTweens.begin(); iter != mTweens.end(); ++iter )
	{
		(**iter).stepTo( time );
	}
	
	for( c_iter iter = mCues.begin(); iter != mCues.end(); ++iter )
	{
		(**iter).stepTo( mCurrentTime );
	}
}

void Sequence::clearSequence()
{
	mTweens.clear();	
}

void Sequence::clearFinishedTweens()
{
	t_iter iter = mTweens.begin();
	
	while (iter != mTweens.end()) {		
		if( (**iter).isComplete() )
		{
			iter = mTweens.erase(iter);
		} else {
			++iter;
		}
	}
}

void Sequence::addTween( TweenRef tween)
{
	mTweens.push_back( tween );
}

TweenRef Sequence::findTween( void *target )
{
	t_iter iter = mTweens.begin();
	while( iter != mTweens.end() ) {
		if( (*iter)->getTargetVoid() == target )
			return *iter;
		++iter;
	}
	
	return TweenRef(); // failed returns null tween
}

void Sequence::removeTween( TweenRef tween )
{
	t_iter iter = std::find( mTweens.begin(), mTweens.end(), tween );
	if( iter != mTweens.end() )
		mTweens.erase( iter );
}


