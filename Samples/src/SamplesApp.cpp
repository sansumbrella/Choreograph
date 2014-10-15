#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"

#include "samples/Samples.h"

using namespace ci;
using namespace ci::app;
using namespace pockets;
using namespace choreograph;
using namespace std;

class SamplesApp : public AppNative {
public:
  void prepareSettings( Settings *settings ) override;
	void setup() override;
	void update() override;

  void loadSample( int index );
private:
  pk::SceneRef            mCurrentScene;
  pk::SceneRef            mPrevScene;
  ch::Timeline            mTimeline;
  ci::Timer               mTimer;
  std::weak_ptr<ch::Cue::Control> mCueControl;
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
  mParams = params::InterfaceGl::create( "Choreograph Samples", ivec2( 240, 100 ) );
  mParams->setPosition( ivec2( getWindowWidth() - 250, 10 ) );
  mParams->addParam( "Sample", SampleNames, &mSceneIndex );
  mParams->addButton( "Next", [this] { loadSample( mSceneIndex + 1 ); } );
  mParams->addButton( "Prev", [this] { loadSample( mSceneIndex - 1 ); } );
  mParams->addButton( "Restart Current", [this] { loadSample( mSceneIndex ); } );
#endif
  // Draw our app first, so samples show up over top.
  getWindow()->getSignalDraw().connect( 0, [this] {
    gl::clear( Color::black() );
#ifndef CINDER_COCOA_TOUCH
    mParams->draw();
#endif
  } );

  loadSample( 0 );
  mTimer.start();
}

void SamplesApp::loadSample( int index )
{
  bool do_animate = (index != mSceneIndex);
  const int start_x = (index < mSceneIndex) ? - getWindowWidth() : getWindowWidth();
  const int vanish_x = - start_x;

  float cooldown = ( mCurrentScene && mCurrentScene->timeline().empty() ) ? 0 : 0.33f;

  if( index < 0 ) { index = SampleList.size() - 1; }
  index %= SampleList.size();

  mSceneIndex = index;
  mSceneName = SampleNames[mSceneIndex];

  console() << "Loading Sample: " << mSceneName << endl;

  if( mCurrentScene && do_animate ) {
    mPrevScene = mCurrentScene;
    // animate off
    mTimeline.apply( mPrevScene->getOffsetOutput() ).hold( cooldown ).then<RampTo>( vec2( vanish_x, 0.0f ), 0.4f, EaseInQuad() )
    .finishFn( [this] ( Motion<vec2> &m ) {
      mPrevScene.reset(); // get rid of previous scene
    } );
    mTimeline.apply( mPrevScene->getAnimationSpeedOutput() ).then<RampTo>( 0, 0.4f );
  }

  mCurrentScene = SampleList[mSceneIndex].second();

  mCurrentScene->setup();
  mCurrentScene->connect( getWindow() );
  mCurrentScene->show( getWindow() );
  // animate current on.
  if( do_animate ) {

    mCurrentScene->setOffset( vec2( start_x, 0.0f ) );
    mCurrentScene->pause();

    mTimeline.apply( mCurrentScene->getOffsetOutput() ).hold( 0.2f + cooldown ).then<RampTo>( vec2( 0 ), 0.66f, EaseOutQuint() )
    .finishFn( [this] ( Motion<vec2> &m ) {
      mCurrentScene->resume();
    } );

  }

  // If there was a previous cue lined up, cancel it.
  auto control = mCueControl.lock();
  if( control ) {
    control->cancel();
  }

  // Load Next Sample Automatically.
  mCueControl = mTimeline.cue( [this] {
    loadSample( mSceneIndex + 1 );
  }, 15.0f ).getControl();
}

void SamplesApp::update()
{
  if( mSceneName != SampleNames[mSceneIndex] ) {
    loadSample( mSceneIndex );
  }

  ch::Time dt = mTimer.getSeconds();
  mTimer.start();
  mTimeline.step( dt );
}

CINDER_APP_NATIVE( SamplesApp, RendererGl )
