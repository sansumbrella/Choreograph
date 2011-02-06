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

void TweenManager::jumpToFrame( int frame )
{
	for ( t_iter t = mTweens.begin(); t != mTweens.end(); ++t) {
		(**t).jumpToFrame(frame);
	}
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

void TweenManager::update()
{
	t_iter iter = mTweens.begin();
	
	while (iter != mTweens.end()) {
		
		(**iter).update();
		
		if( (**iter).isComplete() )
		{
			iter = mTweens.erase(iter);
		} else {
			++iter;
		}
	}
}




