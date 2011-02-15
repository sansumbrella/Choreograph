/*
 *  TweenManager.cpp
 *  BasicTween
 *
 *  Created by David Wicks on 5/27/10.
 *  Copyright 2010 David Wicks. All rights reserved.
 *
 */

#include "TweenManager.h"

using namespace cinder;
using namespace cinder::tween;
typedef std::vector< TweenRef >::iterator t_iter;


TweenManager::TweenManager()
{
	//privately instantiating...
}

void TweenManager::cleanup()
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

TweenManager& TweenManager::instance()
{
	static TweenManager instance;
	return instance;
}

void TweenManager::cancelAllTweens()
{
	// apparently remove_if isn't defined...
	// mTweens.remove_if( Tween::remove );
	
	mTweens.clear();	
}

void TweenManager::addTween( TweenRef tween)
{
	mTweens.push_back( tween );
}

void TweenManager::step()
{	// would like to use getAverageFps, but it doesn't work statically (yet)
	step( 1.0 / app::getFrameRate() );
}

void TweenManager::step( double timestep )
{
	t_iter iter = mTweens.begin();
	
	while (iter != mTweens.end()) {
		
		(**iter).step( timestep );
		
		if( (**iter).isComplete() )
		{
			iter = mTweens.erase(iter);
		} else {
			++iter;
		}
	}
}

void TweenManager::jumpToTime( double time )
{	
	for( t_iter iter = mTweens.begin(); iter != mTweens.end(); ++iter )
	{
		(**iter).jumpToTime( time );
	}
}


