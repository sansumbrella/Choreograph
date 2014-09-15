#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/params/Params.h"

#include "samples/SampleBase.h"
#include "samples/MouseFollow.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class SamplesApp : public AppNative {
public:
  void prepareSettings( Settings *settings ) override;
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

  void buildSampleList();
  void loadSample( int index );
private:
  using SampleList = vector< pair<string, function<SampleRef ()>> >;

  params::InterfaceGlRef  mParams;

  SampleList              mSampleMakers;
  SampleRef               mCurrentSample = nullptr;
  int                     mSampleIndex = 0;
  int                     mPreviousSampleIndex = 0;
};

void SamplesApp::prepareSettings( Settings *settings )
{
  settings->setWindowSize( 1280, 720 );
}

void SamplesApp::setup()
{
  buildSampleList();

  mParams = params::InterfaceGl::create( "Choreograph Samples", ivec2( 200, 300 ) );
  vector<string> names;
  for( auto &sample : mSampleMakers ) {
    names.push_back( sample.first );
  }

  mParams->addParam( "Current Sample", names, &mSampleIndex );
}

void SamplesApp::buildSampleList()
{
  mSampleMakers.push_back( make_pair( "Move to cursor", make_shared<MouseFollow> ) );
  mSampleMakers.push_back( make_pair( "Move to cursor again", make_shared<MouseFollow> ) );
}

void SamplesApp::mouseDown( MouseEvent event )
{
}

void SamplesApp::loadSample( int index )
{
  if( index < 0 ) {
    index = mSampleMakers.size() - 1;
  }
  mSampleIndex = index % mSampleMakers.size();
  mPreviousSampleIndex = mSampleIndex;

  mCurrentSample = mSampleMakers[mSampleIndex].second();
  mCurrentSample->setup();
}

void SamplesApp::update()
{
  if( mPreviousSampleIndex != mSampleIndex ) {
    loadSample( mSampleIndex );
  }
}

void SamplesApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( SamplesApp, RendererGl )
