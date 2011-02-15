/*
 *  TweenManager.cpp
 *  BasicTween
 *
 *  Created by David Wicks on 5/27/10.
 *  Copyright 2010 David Wicks. All rights reserved.
 *
 */

#include "TweenManager.h"
#include "cinder/app/App.h"

using namespace cinder;
using namespace cinder::tween;
typedef std::vector< TweenRef >::iterator t_iter;


TweenManager::TweenManager()
{
	mCurrentTime = 0;
	//privately instantiating...
}

TweenManager& TweenManager::instance()
{
	static TweenManager instance;
	return instance;
}

void TweenManager::step()
{	// would like to use getAverageFps, but it doesn't work statically (yet)
	step( 1.0 / app::getFrameRate() );
}

void TweenManager::step( double timestep )
{
	mCurrentTime += timestep;
	
	for( t_iter iter = mTweens.begin(); iter != mTweens.end(); ++iter )
	{
		(**iter).stepTo( mCurrentTime );
	}
}

void TweenManager::stepTo( double time )
{	
	mCurrentTime = time;
	
	for( t_iter iter = mTweens.begin(); iter != mTweens.end(); ++iter )
	{
		(**iter).stepTo( time );
	}
}

void TweenManager::clearTimeline()
{
	mTweens.clear();	
}

void TweenManager::clearFinishedTweens()
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

void TweenManager::addTween( TweenRef tween)
{
	mTweens.push_back( tween );
}

TweenRef TweenManager::findTween( void *target )
{
	t_iter iter = mTweens.begin();
	while( iter != mTweens.end() ) {
		if( (*iter)->getTargetVoid() == target )
			return *iter;
		++iter;
	}
	
	return TweenRef(); // failed returns null tween
}

void TweenManager::removeTween( TweenRef tween )
{
	t_iter iter = std::find( mTweens.begin(), mTweens.end(), tween );
	if( iter != mTweens.end() )
		mTweens.erase( iter );
}


