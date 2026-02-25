#ifndef ZGOLF_HPP
#define ZGOLF_HPP

#include "objects.hpp"


/*
  
 FRICTION CREEP: add an angle threshold to each surface type where the ball won't move if it zeroed out velocity on shallow enough slant  (or,  fixing centrifugal may fix this, pretty certain FrictionSandbox had no creep)
 -Disregard all centrifugl calculations unless xlatdir exceeds speed threshold?
 
		TO DO:
 =============
 -multiple threads for blur(), fillIn
 -do screen edges properly
 
 -when friction creeping ball (and now deh bc staying aligned with it) sometimes disappear (when tab dropping the ball again, ball is traveling at high rate and the sprite is solid black colored)
 ** ball still freezes in crotch if crotch is made by two different platforms: design platforms without overlap for now
 -centrifugal and friction creep aren't right; not fixed by roll() 1404. (ball could perch on marble of ≈3 degrees; creep doesn't seem to respect muS value  unless it's because gravity factor isn't proportionate)
 -fine tune surface physics after fixing centrifugal, friction creep
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

class SFGameWindow;
class TimedEventManager;

class State {
public:
	
	enum Mode { design, play, menu };
 
	static State* getSelf () { return instance_; }
	
	void onCreate () ;
	
	void loadFonts () ;
	
	void loadTextures () ;
	
	void addTexToMap (pair<string, string>) ;
	
	void loadSounds () ;
	
	void resetGame () ;
	
	void draw () ;
	
	void loadCourses () ;
	
	void loadCourse (Course&) ;
	
	void loadNextHole () ;
	
	void loadPlatforms (string fname = "platforms") ;
	
	void onMouseDown (int x, int y) ;
	
	void onMouseUp (int x, int y) ;
	
	void onKeyPress (Keyboard::Key) ;
	
	void onKeyRelease (Keyboard::Key) ;
	
	void switchToPlay () ;
	
	void switchToDesign () ;
	
	void update (const Time& time) ;
	
	void playUpdate (const Time& time) ;
	
	void fly (float) ;
	
	void roll (float, GroundSegment* = nullptr) ;
	
	void launch () ;
	
	void handleSwing () ;
	
	void ballInHole () ;
	
	void updateGuide () ;
	
	void updatePowerBar (float) ;
	
	void startRoll (GroundSegment* seg)
	{
		rolling = true;
		gSeg = seg;
	}
	
	void endRoll () ;
	
	void zeroOutVelocity () ;
	
	void disableShooting () ;
	
	void startNewShotTimer () ;
 
	void assembleSprite (string) ;
	
	
	
	void loadToolbarButtons () ;
	
	void loadPlatformData (string fname = "platforms") ;

	void designKeyPress (Keyboard::Key) ;

	void designClick (int, int) ;

	void designUpdate () ;

	void designDraw () ;

	bool finishGround (EditorPlatform*, bool makeNew = true) ;

	void clearMap () ;

	void redrawSpline () ;

	bool maybeEraseSelectedVert () ;
	
	void activateSelectButton () ;

	bool handleTextEvent(Event&) ;

	void saveHole () ;
	
	void updateHoleAndTee (pair<Vert*, Vert*>) ;
	
	void newPlatform () ;
	
	void deactivateCurTbox () ;
	
	void setCurPlat (EditorPlatform*) ;

	
	void menuDraw() ;
	
	void menuClick(int x, int y) ;
	

	static vector<pair<string, string>> surfaceTypeList;
	static vector<pair<string, string>> surfaceEndList;
	static vector<pair<string, string>> fillTypeList;
	static map<string, SurfacePhysics>	physicsMap;
	Mode        mode = menu;

	/* Design mode members */
	Textbox					filenameTbox;
	Textbox					fillInfoTbox;
	Textbox*				activeTbox = nullptr;
	Vert*                   gHighlighted = nullptr;
	Vert*                   gClicked = nullptr;
	vector<EditorPlatform>  curPlatforms;
	EditorPlatform*			curPlat = nullptr;
	string					curSurfType = "grass";
	ToolWindow				toolWin {vecf(100, 800)};
	string					curTool;
	
	Sprite					bkgdSpr;
	vector<CourseButton>	courseButtons;
	Text					menuTitle;

	Text					flagTxt
							, statsTxt
	;
 
	vector<Course>			courses;
    vector<Platform>        platforms;
	Course*					curCourse;
	CourseHole*				curHole;
    VertexArray             guideline {Lines}
							, powerBarOutline {LineStrip}
							, powerBar {TriangleStrip}
	;
	
	GroundSegment*          gSeg = nullptr;
	VSprite             	ball;
	Sprite                  deh;
	Sprite					flag;
	Sprite					curHoleSprite;
	Hole					hole;

    float       power = 0
                , angle = 315
                , angleRate = .5
                , gravity = .25
				, avgSeg = 25
                , minseg = 15
				, speedClamp = .1
				, fracRemEps = .01
				, snapToEndEps = .08
				, ballRadius
    ;
    vecF        vGravity {0, gravity}
	;
    bool        pullingBack
                , ballActive
				, putting
                , rolling
				, inCrotch
				, onCusp
				, powerRising
				, teeingOff
    ;
	CrotchInfo				crotchInfo;
    
    Sprite                  clouds[10];
    float                   cloudVels[10];
    
    RenderTexture           rt;
    Sprite                  rts;
    Texture                 rtTx;
	ZImage					rtImg;
	

	
	vector<AnimFrame>		swingFrames; //@kludgeAnim
	int						curFrameNum; //@kludgeAnim
	void setDehFrame (int) ; //@kludgeAnim
	void loadAnimFrames () ; //@kludgeAnim
	void startDownswing () ;
	
	
	static State*			instance_;
	RenderWindow*   w;
	SFGameWindow*	gw;
	TimedEventManager*	timedMgr;
	int             mx = 0,
					my = 0,
					mxOld = 0,
					myOld = 0;
	Text            mouseTxt;
  
	map<string, Font> 			fontMap;
	static const vector<pair<string, string>>
								fontList;

	map<string, Texture> 		txMap;
	static vector<pair<string, string>>
								txList;

	vector<SoundBuffer> 		buffers;
	map<string, Sound> 			soundMap;
	static const vector<pair<string, string>>
								soundList;

	
	
	//DEBUG ///////////////////////////////////////////////
	string					curPlatFile;

	string dbgMsg;
	uint frameCounter = 0;
	uint storedFrame = 0;
	bool pauseAfterDraw = false;
	bool enterBreakpoints = false;
	class RR : public RectangleShape
	{ public: RR(){ setSize({150,150});setPosition(0,400);} };
	RR rr{};
	
	class RRR : public RectangleShape
	{ public: RRR(){ setSize({150,150});setPosition(0,600);} };
	RRR rrr{};
	LineSegment lastPath;
	RenderTexture drt;
	bool firstCollision = false;
//	GroundSegment gs {{800,500}, {850,500}};
	VertexArray gsva {Lines};
	
	void drtDraw(const Drawable& d)
	{
		drt.draw(d);
		drt.display();
//		rts.setTexture(drt.getTexture());
	}
	
	void drtDraw(const vecf& pos, const vecf& pv, Color c = PURPLE)
	{
		RectangleShape r;
		r.setSize({pv.x, 2});
		r.setPosition(pos);
		r.setFillColor(c);
		r.setRotation(pv.y);
		drtDraw(r);
	}
	
	
	void a()
	{
		Texture tx;
		tx.loadFromFile("resources/china.png");
		ZImage zim {tx.copyToImage()};
		zim.convertToRetroColor();
		zim.saveToFile("resources/retroExc.png");
	}
	
	void makeBlotchTx()
	{
		Texture tx;
//		tx.loadFromFile("resources/blotch.png");
		tx.loadFromFile("resources/blotch2.png");
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
		
		zim.saveToFile("resources/newblotches.png");
	}

	void dbgSky(Color c);
	
	void checkForShortSegs(float segLength)
	{
		ifstream ifs {"levels/platforms.txt"};
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
// ///////////////////////////////
   
}; //end class State
#endif
