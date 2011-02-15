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
	//update all tweens in progress
//	TweenManager::instance().update( 1 / 60.0f );
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
	TweenRef tween = TweenRef( new Tween<Vec3f>( &mPos, randomPos, 0.0, 2.0 ) );
	TweenManager::instance().addTween( tween );
//	TweenManager::instance().add( &mPos, randomPos, 2.0 );
	
	// Tween our floats
	randomPos = Vec3f(Rand::randFloat(getWindowWidth()), Rand::randFloat(getWindowHeight()), 0.0f);
	TweenManager::instance().add( &mX, randomPos.x, 2.0 );
	TweenManager::instance().add( &mY, randomPos.y, 2.0 );
}

void BasicTweenApp::tweenToMouse()
{
	// Move our properties to the mouse position with with different easing
	
	Vec3f mousePos = Vec3f( getMousePos().x, getMousePos().y, 0.0f );
	// Tween a Vec3f all at once with custom easing
	TweenRef newTween = TweenManager::instance().replace( &mPos, mousePos, 1.25, Back::easeOut );
	
	// Tween our floats
	TweenManager::instance().replace( &mX, mousePos.x, 2.0, Back::easeInOut );
	TweenManager::instance().replace( &mY, mousePos.y, 1.5, Back::easeInOut );
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
