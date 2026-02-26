
/*
  
 FRICTION CREEP: add an angle threshold to each surface type where the ball won't move if it zeroed out velocity on shallow enough slant  (or,  fixing centrifugal may fix this, pretty certain FrictionSandbox had no creep)
 -Disregard all centrifugl calculations unless xlatdir exceeds speed threshold?
 -replace centrif w note of upsidedownness, angle + speed?
 
		TO DO:
 =============
 -multiple threads for blur(), fillIn (but heard that pixel work isn't good to divide between threads?)
 -do screen edges properly
 
 -when friction creeping ball (and now deh bc staying aligned with it) sometimes disappear (when tab dropping the ball again, ball is traveling at high rate and the sprite is solid black colored)
 ** ball still freezes in crotch if crotch is made by two different platforms: design platforms without overlap for now
 -centrifugal and friction creep aren't right; not fixed by roll() 1404. (ball could perch on marble of ≈3 degrees; creep doesn't seem to respect muS value  unless it's because gravity factor isn't proportionate)
 -fine tune surface physics after fixing centrifugal & friction creep
 REFACTOR ALL LINE CODE TO USE X FOR NEAR VERTICAL
 !- another NAN/INF ball position when putting on a gentle convex/concave curve
 !- another roll() stack overflow in acute crotch


 
 
 
	WISH LIST:
 ==============
 -graphic-only "end caps" where e.g. grass overhangs a stone wall (rather than visually bisecting
 -draw the surface sprites following the splines in editor mode

 -sophisticate rolling into hole checking/jumping
 -may need to clear events and reset animframes on loadNextHole?
 -is it important to erase platform from vector if command-deleting a vert

 -color picker
 -water hazards;  wind
 -panning up and over
 -in level.txt only store editor verts and their controls: only divide the curves into segments in loadPlatforms for physics and drawing
 -Hopscotch-style non-collision scenery
 -some kind of indicator whether an angle will roll or bounce
 -set par for hole in editor
 -ball bonks deh on head
 -deh jumps on ballInHole
 
 
	REMINDERS:
===============
! image for fill can't have any transparent pixels
! ZImage fill may be susceptible to getting start points that cause infinite loops. If problem recurs, add fillStartPt to (editor)Platform and be able to set it with mouseclick while key held
- zone off segs if performance is issue, or break up large plats into overlapping parts
{ controls are almost guaranteed to invalidate `parent` after Vert deletion/insertion? use smartptrs or have a routine to reassign ptr values after vector elems have been moved.  IF ADDING new points to an already completed platform, either remove shrinktofit or make sure pointers are revalidated because the new points should move p.verts in memory
}




 ===========PHYSICS TWEAKS/QUIRKS

 **note: adding "wind" can cause ball to go through crotches
 utilize segToIgnore when doing convex roll, but accounting for gravity changing the direction right on that point
 doublecheck gravity overcoming inertia right on junction points, both for freezing or for rolling under ground
 
 do onCusp symmetrical to inCrotch? change crotchInfo to junctInfo.isConcave
 
 revisit whether sinValue should be used for rebounds where surface doesn't angle back down
 -keep some centrifugal on convex transition? (residual gravity)
 -filter segments in loadPlatforms so no two consecutive segs are within certain fraction of degrees of same angle? (could be causing roll fallthrough)
 ==============
*/

#ifndef ZGOLF_HPP
#define ZGOLF_HPP

#include "vsprite.hpp"
#include "resourcemanager.hpp"
#include "timedeventmanager.hpp"
#include <variant>

class State;

/* These are all consts in practice, but leaving them untagged
 * while still sometimes adjusting the values with in-game
 * key press
 */
inline float 			muK = .04;	// kinetic friction
inline float 			muS = .05;	// static friction
inline float			bounceLoss = .35;
inline float       		bounceClamp = 1.3;
inline float			maxAngForRoll = 36;
inline float			maxAngForRollCvx = 25;
inline float			minAngForIronShot = 5;
inline float			ironMinDevFromVertical = 7;
inline float			convexRollClamp = 1.4;
inline float			centrifugalDecmFactor = 1;
/* If using this factor: too high a value (even .3) causes ball to "dig" in
 * to the ground too aggressively, stopping the ball from climbing upward
 * curvature. Too low, and the ball falls too easily off loops, resulting
 * in unrealistic looking bounces off the angle of the next seg
 */
inline float 			centrifugalIncmFactor = .24;

inline int         			nextSegID = 1;
inline constexpr float      powerRate = 1.5;
inline constexpr float      dfltMaxPower = 20;
inline constexpr float      maxPuttPower = 20;
inline constexpr float      maxPct = 100;


// DEBUG: was printing angle numbers for ground segments
inline float stagger ()
{
	static float cur = 20;
	float ret = cur;
	cur += 20;
	if (cur > 100)
		cur = 20;
	return ret;
}

//#define DBG	// activates some code blocks


#include "GroundSegment.hpp"
#include "Vert.hpp"
#include "platforms.hpp"
#include "objects.hpp"
#include "ToolWindow.hpp"


class FullscreenOnlyApp;

class State
{
public:
	enum Mode { design, play, menu };
 
	static State* getSelf () { return instance_; }
	
	void onCreate () ;
	
	bool handleTextEvent(Event&) ; // in designMethods.cpp
	
	void onMouseDown (int x, int y) ;
	
	void onMouseUp (int x, int y) ;
	
	void onKeyPress (Keyboard::Key) ;
	
	void onKeyRelease (Keyboard::Key) ;
	
	void update (const Time& time) ;
	
	void draw () ;
	
	RenderWindow*   	rwin;
	FullscreenOnlyApp*	app;
	TimedEventManager*	timedMgr;
	vecI				mouseVec
						, oldMouse
	;
	
	bool pauseAfterDraw = false;	// DEBUG
	
private:
	
	const float			gravity = .25
						, angleRate = .5
						, avgSeg = 25
						, minseg = 15
						, speedClamp = .1
						, fracRemEps = .01
						, snapToEndEps = .08
	;
	const vecF			vGravity {0, gravity};

	void loadAnimFrames (); //@kludgeAnim
	
	void resetGame ();
	
	void loadCourses ();
	
	void loadCourse (Course&);
	
	void loadNextHole ();
	
	void loadPlatforms (string fname = "platforms");

	void menuDraw();
	
	void menuClick(int x, int y);

	
/* Play mode methods */
	void switchToPlay ();
	
	void assembleSprite (string);
	
	void playUpdate (const Time& time);
	
	void updateGuide ();
	
	void handleSwing ();

	void updatePowerBar (float);
	
	void startDownswing (); //@kludgeAnim
	
	void setDehFrame (int); //@kludgeAnim
	
	void launch ();
	
	void fly (float);
	
	void roll (float, GroundSegment* = nullptr);
	
	void startRoll (GroundSegment* seg)
	{
		rolling = true;
		gSeg = seg;
	}
	
	void endRoll ();
	
	void zeroOutVelocity ();
	
	void disableShooting ();
	
	void startNewShotTimer ();
 
	void ballInHole ();
	
	
/* Design mode methods (defs in designMethods.cpp) */
	void switchToDesign ();
	
	void loadToolbarButtons ();
	
	void loadPlatformData (string fname = "platforms");

	void designKeyPress (Keyboard::Key);

	void designClick (int, int);

	void designUpdate ();

	void designDraw ();

	bool finishGround (EditorPlatform*, bool makeNew = true);

	void clearMap ();

	bool maybeEraseSelectedVert ();
	
	void activateSelectButton ();

	void updateHoleAndTee (pair<Vert*, Vert*>);
	
	void newPlatform ();
	
	void deactivateCurTbox ();
	
	void setCurPlat (EditorPlatform*);

	void saveHole ();
	
	
	static State*			instance_;
	Mode        			mode = menu;

	static vector<pair<string, string>> 	surfaceTypeList;
	static vector<pair<string, string>> 	surfaceEndList;
	static vector<pair<string, string>> 	fillTypeList;
	static map<string, SurfacePhysics>		physicsMap;
	
	vector<Course>			courses;
	vector<Platform>        platforms;
	string					curPlatFile;

/* Menu members */
	Sprite					bkgdSpr;
	vector<CourseButton>	courseButtons;
	Text					menuTitle;

/* Play mode members */
	Sprite                  deh;
	VSprite             	ball;
	Hole					hole;
	Sprite					flag;
	Sprite                  clouds[10];
	Sprite					curHoleSprite;
	Sprite					trajecSpr;
	VertexArray             guideline {Lines}
							, powerBarOutline {LineStrip}
							, powerBar {TriangleStrip}
	;
	Text					flagTxt
							, statsTxt
	;
	Texture                 trajecTx;
	ZImage					trajecImg;
	
	Course*					curCourse;
	CourseHole*				curHole;
	GroundSegment*          gSeg = nullptr;
	CrotchInfo				crotchInfo;
	vector<AnimFrame>		swingFrames; //@kludgeAnim

	float                   cloudVels[10];
	float       			power = 0
                			, angle = 315
							, ballRadius
    ;
	int						curFrameNum; //@kludgeAnim
    bool					ballActive
							, pullingBack
							, putting
                			, rolling
							, inCrotch
							, onCusp
							, powerRising
							, teeingOff
    ;
    
/* Design mode members */
	ToolWindow				toolWin {vecf(100, 800)};
	Textbox					filenameTbox;
	Textbox					fillInfoTbox;
	vector<EditorPlatform>  curPlatforms;
	Textbox*				activeTbox = nullptr;
	Vert*                   gHighlighted = nullptr;
	Vert*                   gClicked = nullptr;
	EditorPlatform*			curPlat = nullptr;
	string					curSurfType = "grass";
	string					curTool;


	
	
/*//////   DEBUG / TEMP   ////////*/
	
	/* Sky color indicates which control blocks are being entered
	 * in fly() and roll()
	 */
	void dbgSky(Color c);
	
	void drtDraw(const Drawable& d)
	{
		drt.draw(d);
		drt.display();
		//		rts.setTexture(drt.getTexture());
	}
	
	/* Draw directional lines from an origin */
	void drtDraw(const vecf& pos, const vecf& pv, Color c = PURPLE)
	{
		RectangleShape r;
		r.setSize({pv.x, 2});
		r.setPosition(pos);
		r.setFillColor(c);
		r.setRotation(pv.y);
		drtDraw(r);
	}
	
	void testRetro()
	{
		Texture tx;
		tx.loadFromFile(resourcePath() / "images" / "china.png");
		ZImage zim {tx.copyToImage()};
		zim.convertToRetroColor();
		zim.saveToFile(resourcePath() / "images" / "retroExc.png");
	}
	
	void makeBlotchTx()
	{
		Texture tx;
		//		tx.loadFromFile((resourcePath() / "images" / "blotch.png").string());
		tx.loadFromFile((resourcePath() / "images" / "blotch2.png").string());
		Sprite s {tx};
		Color anchor = Color(210, 215, 215);
		drt.clear(anchor);
		auto func = [&] (int iterCt, float scMin, float scMax) {
			forNum(iterCt) {
				s.sP(randRange(1770), randRange(1250));
				auto scx = randFloat(scMin, scMax);
				auto scy = randFloat(scMin, scMax);
				if (flipCoin())
					scx *= -1;
				if (flipCoin())
					scy *= -1;
				s.setScale(scx, scy);
				s.setColor(colorDevLockHue(anchor, 2));
				s.setRotation(czdg(randFloat(0, 360)));
				
				drt.draw(s);
			}
		};
		func(500, 2, 4);
		func(800, .5, 2);
		func(800, .5, 1);
		func(800, .2, .5);
		func(4200, .05, .2);
		drtDraw(s);
		
		ZImage zim {drt.getTexture().copyToImage()};
		zim.blur();
		drt.clear();
		Texture tx2;
		tx2.loadFromImage(zim);
		Sprite s2(tx2);
		drtDraw(s2);
		rts.setTexture(drt.getTexture());
		
		zim.saveToFile((resourcePath() / "images" / "newblotches.png").string());
	}
	
	void checkForShortSegs(float segLength)
	{
		ifstream ifs {resourcePath() / "levels" / "platforms.txt"};
		ofstream ofs {"abc.txt"};
		string line;
		while (getline(ifs, line)) {
			vecf start, end;
			stringstream ss(line);
			string tok;
			ss >> tok;
			if (tok == ':') {
				ofs << line << '\n';
				continue;
			}
			start.x = stof(tok);
			ss >> tok;
			start.y = stof(tok);
			ss >> tok;
			end.x = stof(tok);
			ss >> tok;
			end.y = stof(tok);
			ofs << line << '\n';
			if (hyp(start, end) < segLength)
				ofs << "SHORT\n";
		}
		ifs.close();
		ofs.close();
	}
	
	void testCcvCvx ()
	{
		static bool reset = false;
		for (auto& p : platforms)
			for (auto& s : p.segs) {
				if (
					!s.concaveFromPrev
					//								!reset && s.concaveToNext && angleBetween(s.angle, s.next->angle) < maxAngForRoll
					//								|| !s.concaveToNext && angleBetween(s.angle, s.next->angle) < maxAngForRollCvx
					)
					s.spr.setColor(Color::Black);
				else s.spr.setColor(Color::White);
			}
		reset = !reset;
	}
	
	class RR : public RectangleShape
	{ public: RR(){ setSize({150,150});setPosition(0,400);} };
	RR rr{};
	
	class RRR : public RectangleShape
	{ public: RRR(){ setSize({150,150});setPosition(0,600);} };
	RRR rrr{};

	Sprite                  rts;
	VertexArray 			gsva {Lines};
	Text            		mouseTxt;
	RenderTexture 			drt;

	//	GroundSegment gs {{800,500}, {850,500}};
	LineSegment 			lastPath;
	string 					dbgMsg;
	uint 					frameCounter = 0;
	uint 					storedFrame = 0;
	bool 					enterBreakpoints = false;
	bool 					firstCollision = false;
	
}; //end class State

#endif
