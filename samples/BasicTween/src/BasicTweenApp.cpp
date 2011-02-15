#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"

#include "Tweening.h"

using namespace ci;
using namespace ci::app;
using namespace ci::tween;

class BasicTweenApp : public AppBasic {
public:
	void prepareSettings(Settings *settings);
	
	void setup();
	void update();
	void draw();
	void shutdown();
	
	void keyDown( KeyEvent event );
	void keyUp( KeyEvent event );
	
	void playRandomTween();
	void tweenToMouse();
	
	void mouseDown( MouseEvent event );
	
	void resize( int width, int height );
	
	void fileDrop( FileDropEvent event );
	
private:	
	float mX, mY;
	Vec3f mPos;
	ColorA mColor;
};

void BasicTweenApp::prepareSettings(Settings *settings)
{
	settings->setWindowSize(600,400);
	settings->setTitle("BasicTween");
}

void BasicTweenApp::setup()
{	
	mX = getWindowWidth()/2;
	mY = getWindowHeight()/2;
	mPos = Vec3f(0,0,0);
	
	mColor = ColorA( 0.5, 0.55, 0.52, 1.0 );
	playRandomTween();
	
}

void BasicTweenApp::update()
{
	// step our animation forward
	TweenManager::instance().step( 1.0 / 60.0 );
//	TweenManager::instance().step();
}

void BasicTweenApp::draw()
{
	gl::clear(mColor);
	
	gl::color(Color::black());
	gl::drawSolidCircle( Vec2i( mX, mY ), 20.0f );
	
	gl::color(Color::white());
	gl::drawSolidCircle( Vec2i(mPos.x, mPos.y), 15.0f );
	
}

void BasicTweenApp::resize(int width, int height)
{
	
}

//MouseEvents
void BasicTweenApp::mouseDown( MouseEvent event )
{
	tweenToMouse();
}

void BasicTweenApp::playRandomTween()
{
	// Tween a Vec3f
	Vec3f randomPos = Vec3f(Rand::randFloat(getWindowWidth()), Rand::randFloat(getWindowHeight()), 0.0f);
	
	// Create our tween (will try to make this prettier in the future)
	TweenRef tween = TweenRef( new Tween<Vec3f>( &mPos, randomPos, 2.0 ) );
	TweenManager::instance().addTween( tween );
	
	// Tween our floats
	randomPos = Vec3f(Rand::randFloat(getWindowWidth()), Rand::randFloat(getWindowHeight()), 0.0f);
	tween = TweenRef( new Tween<float>( &mX, randomPos.x, 2.0 ) );
	TweenManager::instance().addTween( tween );
	
	tween = TweenRef( new Tween<float>( &mY, randomPos.y, 2.0 ) );
	TweenManager::instance().addTween( tween );
}

void BasicTweenApp::tweenToMouse()
{
	// Move our properties to the mouse position with with different easing
	
	Vec3f mousePos = Vec3f( getMousePos().x, getMousePos().y, 0.0f );
	// Tween a Vec3f all at once with custom easing
	TweenRef tween = std::shared_ptr<Tweenable>( new Tween<Vec3f>( &mPos, mousePos, 1.25, Back::easeOut ) );
	TweenManager::instance().addTween( tween );
	
	
	// Tween our floats
	tween = TweenRef( new Tween<float>( &mX, mousePos.x, 2.0, Back::easeInOut ) );
	TweenManager::instance().addTween( tween );
	
	tween = TweenRef( new Tween<float>( &mY, mousePos.y, 1.5, Back::easeInOut ) );
	TweenManager::instance().addTween( tween );
}

//KeyEvents
void BasicTweenApp::keyDown( KeyEvent event )
{
	switch( event.getChar() ){
		default:
			playRandomTween();
		break;
	}
}

void BasicTweenApp::keyUp( KeyEvent event )
{
	switch( event.getChar() ){
		default:
		break;
	}
}

void BasicTweenApp::fileDrop( FileDropEvent event )
{
	
}

void BasicTweenApp::shutdown()
{
//	TweenManager::instance().cancelAllTweens();
}


// This line tells Cinder to actually create the application
CINDER_APP_BASIC( BasicTweenApp, RendererGl )
