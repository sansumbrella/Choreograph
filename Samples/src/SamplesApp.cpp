#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include "samples/Samples.h"

using namespace ci;
using namespace ci::app;
using namespace pockets;
using namespace std;

class SamplesApp : public AppNative {
public:
  void prepareSettings( Settings *settings ) override;
	void setup() override;
	void update() override;

  void loadSample( int index );
private:
  pk::SceneRef            mCurrentScene;
  int                     mSceneIndex = 0;
  string                  mSceneName;
  params::InterfaceGlRef  mParams;
};

void SamplesApp::prepareSettings( Settings *settings )
{
  settings->setWindowSize( 1280, 720 );
  settings->enableMultiTouch();
  settings->disableFrameRate();
}

void SamplesApp::setup()
{
#ifndef CINDER_COCOA_TOUCH
///*
  mParams = params::InterfaceGl::create( "Choreograph Samples", ivec2( 200, 200 ) );
  mParams->addParam( "Sample", SampleNames, &mSceneIndex );
  mParams->addButton( "Next", [this] { loadSample( mSceneIndex + 1 ); } );
  mParams->addButton( "Prev", [this] { loadSample( mSceneIndex - 1 ); } );
  mParams->addButton( "Reload", [this] { loadSample( mSceneIndex ); } );
// */
#endif
  // Draw our app first, so samples show up over top.
  getWindow()->getSignalDraw().connect( 0, [this] {
    gl::clear( Color::black() );
#ifndef CINDER_COCOA_TOUCH
    mParams->draw();
#endif
  } );

  loadSample( 0 );
}

void SamplesApp::loadSample( int index )
{
  index %= SampleList.size();
  if( index < 0 ) { index = SampleList.size() - 1; }
  mSceneIndex = index;
  mSceneName = SampleNames[mSceneIndex];

  console() << "Loading Sample: " << mSceneName << endl;
  mCurrentScene = SampleList[mSceneIndex].second();

  mCurrentScene->setup();
  mCurrentScene->connect( getWindow() );
  mCurrentScene->show( getWindow() );
}

void SamplesApp::update()
{
  if( mSceneName != SampleNames[mSceneIndex] ) {
    loadSample( mSceneIndex );
  }
}

CINDER_APP_NATIVE( SamplesApp, RendererGl( RendererGl::Options().antiAliasing( RendererGl::AA_NONE ) ) )
