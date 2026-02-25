
#ifndef sfmlApp_hpp
#define sfmlApp_hpp


#include "zgolf.hpp"
#include "timedeventmanager.hpp"

inline const string defaultTitle { "ZGolf" };
inline const string iconPath { "resources/golfdeh.png" };


class SFGameWindow {

	friend class Game;
public:
    
    SFGameWindow ();

    SFGameWindow (const string& title, const vecU& size);
    
	~SFGameWindow () { destroy(); }
    
	void draw (Drawable& d) { window.draw(d); }
    
	void beginDraw () { window.clear(redrawColor); }
    
	void endDraw () { window.display(); }
    
	bool isDone () { return _isDone; }
    
	bool isFullscreen () { return _isFullscreen; }
    
    bool isStretched () { return _isStretched; }
    
	bool isFocused () { return _isFocused; }
    
	vecU getWindowSize () { return windowSize; };
    
	RenderWindow* getRenderWindow () { return &window; };
    
	void close () { _isDone = true; };
    
	void toggleFullscreen ();
	
	void toggleStretchGraphics ();
    
    void setToggledView (bool);
	
	void setRedrawColor(const Color& c) { redrawColor = c; }
     
    
    vecf 					screenOffsetFrom1440x900;
	Color                   redrawColor { SKYBLUE };
    
private:
    
    void destroy () { window.close(); };
    
	void setup (const string& title, const vecU& size);
    
	void create ();
    
    Image                   icon;
    RenderWindow            window;
    vecU                    windowSize;
    static const int        defaultWidth { 1280 };
    static const int        defaultHeight { 720 };
    string                  windowTitle;

    bool                    _isDone;
    bool                    _isFullscreen;
    bool                    _isFocused;
    bool                    _isStretched;
};




class Game {
public:
    
    Game ();
       
	void update ();
    
	void render () {
        window.beginDraw();
        state.draw();
        window.endDraw();
		
		// /////DEBUG
		if (state.pauseAfterDraw) {
			while (!iKP(B))
				state.pauseAfterDraw = false;
		}
		// //////
	}
    
	void lateUpdate () { restartClock(); }
    
	SFGameWindow* getWindow () { return &window; };
    
	Time getElapsed () { return elapsed; };
    
	void restartClock () { elapsed += clock.restart(); };

private:

    SFGameWindow     window;
    TimedEventManager      timedMgr;
    State            state;
    Clock            clock;
    Time             elapsed;
};




#endif /* sfmlApp_hpp */
