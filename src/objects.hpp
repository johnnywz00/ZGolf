//
//  objects.hpp
//  ZGolf
//
//  Created by John Ziegler on 1/9/25.
//  Copyright © 2025 John Ziegler. All rights reserved.
//

#ifndef objects_hpp
#define objects_hpp

#include "vsprite.hpp"
#include <variant>

//#define DBG

class GroundSegment;
struct EditorPlatform;
struct EditorGroundSeg;

inline float 			SCRW
						, SCRH
						, SCRCX
						, SCRCY
;



inline float 			muK = .04;
inline float 			muS = .05;
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




	// DBG: was printing angle numbers for ground segments
inline float stagger () {
	static float cur = 20;
	float ret = cur;
	cur += 20;
	if (cur > 100)
		cur = 20;
	return ret;
}

class GroundSegment;
class State;

class GroundSegment
{
public:
	GroundSegment (const vecF& s, const vecF& e, float ballRadius = 4.5)
		: lseg(s, e)
		, start(s)
		, end(e)
	{
		mid = lseg.mid;
		length = lseg.length;

			/* angle is vector direction from start point to end point.
			 * the segment "faces" +90 to this, thus "ground" ranges from
			 * >90 to <270, otherwise "wall/ceiling"
			 */
		angle = lseg.angle;
		normal = lseg.normal;
		oppAngle = lseg.oppAngle;
		oppNormal = lseg.oppNormal;
		minx = lseg.minx;
		maxx = lseg.maxx;
		miny = lseg.miny;
		maxy = lseg.maxy;
		bounds = lseg.bounds;
		slope = lseg.slope;
		yIcpt = lseg.yIcpt;
		xIcpt = lseg.xIcpt;
		
		if (lseg.isVertical())
			isVertical = true;
		else if (lseg.isHorizontal())
			isHorizontal = true;
		if (angle > 90 && angle < 270)
			facesUp = true;
		
		collisionCheckLine = LineSegment(lseg.pt1 + pVec(ballRadius, normal), lseg.pt2 + pVec(ballRadius, normal));
		
		id = nextSegID++;
		
		txt = Text();
//		txt.setString(fS(angle, 1));
		txt.setString(tS(id));
		txt.setCharacterSize(13);
		txt.setFillColor(Color::Black);
//		float txtY = mid.y - 20;
//		if (length < 50)
//			txtY = (facesUp ?  mid.y - stagger() : mid.y + stagger());
//		txt.sP(mid.x - 50, txtY);
		centerOrigin(txt); // //
		txt.sP(mid); // //

		//DBG
		vecf nend = mid + pVec(20, normal);
		n.append(VXC(mid.x, mid.y, Color::Red));
		n.append(VXC(nend.x, nend.y, Color(255, 200, 200)));
	}
	
	virtual void initSprite (const Texture& tx)
	{
		spr = Sprite(tx);
		spr.setTextureRect(IntRect(
						   0, 0, length,
						   max( int(spr.getTexture()->getSize().y), 7)));
		spr.setOrigin(0, spr.gLB().height - 4.5); // 4.5 is ballRadius
		spr.sP(start);
		spr.sRot(angle);
	}
	
	void setConcavityToNeighbors ()
	{
		concaveToNext = clockwiseOf(next->angle, angle);
		concaveFromPrev = clockwiseOf(angle, prev->angle);
		
		/* When constructed, a segment's check line is the same length as the
		 * segment. Now we'll extend the ends of any checklines where there are
		 * convex corners. We end up creating the check lines a second time over,
		 * but it's convenient to have the originals in place before this in
		 * order to get the intersection points.
		 */
//		auto pt1 = collisionCheckLine.pt1;
//		auto pt2 = collisionCheckLine.pt2;
//		if (!concaveFromPrev)
			auto
			pt1 = collisionCheckLine.intersectionPointWith(prev->collisionCheckLine.line);
//		if (!concaveToNext)
			auto
			pt2 = collisionCheckLine.intersectionPointWith(next->collisionCheckLine.line);
		collisionCheckLine = LineSegment(pt1, pt2);
		checkLineIsctPrev = pt1;
		checkLineIsctNext = pt2;
	}
	
	bool ptIsNormalFromCheckline(const vecf& pt)
	{
		vecf pdif = toPolar(pt - collisionCheckLine.pt1);
		return clockwiseOf(pdif.y, angle);
	}
	
	LineSegment 	lseg;
	LineSegment		collisionCheckLine;
	Sprite 			spr;
	string			surfaceType = "grass";
	Text 			txt;
	FloatRect 		bounds;
	GroundSegment*	prev {nullptr};
	GroundSegment*	next {nullptr};
	vecF 			start;
	vecF 			end;
	vecF 			mid;
	vecf			checkLineIsctPrev;
	vecf			checkLineIsctNext;
	float 			angle
					, oppAngle
					, normal
					, oppNormal
					, length
	;
	float 			slope
					, yIcpt
	;
	float 			minx
					, maxx
					, miny
					, maxy
	;
	float 			xIcpt;
	float 			muK = ::muK;  //.01;
	float 			muS = ::muS;  //.1;
	float 			bounceLoss = ::bounceLoss;  // used as mag * (1 - bounceLoss)
	int 			id = 0;
	bool 			isVertical = false
					, isHorizontal = false
					, facesUp = false
					, concaveFromPrev = false	// set by platform loading
					, concaveToNext = false
	;
	
	
	//DBG
	VertexArray     n {Lines};
};




struct PlatFillInfo
{
	PlatFillInfo ()
	{
		type = FillType::colorDev;
		arg = ColorDevInfo(Color(207, 163, 0), 14, 1);
	}
	PlatFillInfo (string fname) : arg(fname) { type = FillType::imagePixs; }
	PlatFillInfo (const Color& c, int dev = 0, int reps = 1) : arg(ColorDevInfo(c, dev, reps)) { type = FillType::colorDev; }
	
	enum class FillType { imagePixs, colorDev };
	
	struct ColorDevInfo {
		ColorDevInfo (const Color& col, int d = 0, int reps = 1)
			: c(col)
			, dev(d)
			, blurRepetitions(reps)
		{ }
		Color c;
		int dev;
		int blurRepetitions;
	};
	
	FillType type;
	std::variant<string, ColorDevInfo> arg;
};


struct Platform {
	vector<GroundSegment>	segs;
	TransformableVxArray 	va {LineStrip};
	// If dynamic elements added, Platforms may be drawn individually
	// again instead of being amalgamated into one static sprite
//	Sprite 					s;
//	Texture 				tx;
	Color					fillColor {Color(207, 163, 0)};
	PlatFillInfo			fillInfo;
};



struct EditorGroundSeg : public GroundSegment
{
	EditorGroundSeg () : GroundSegment(vecf(0,0), vecf(0,1)) { }
	EditorGroundSeg (const vecF& s, const vecF& e)
		: GroundSegment(s, e) {	}
	
	void initSprite (const Texture& tx) override
	{
		GroundSegment::initSprite(tx);
		spr.setColor(Color(255, 255, 255, 120));
	}
	
	string surfaceType;
	bool hasHole = false;
	bool hasTee = false;
};



class Vert
{
public:
	Vert() { setup(); }
	
	Vert(const vecF& pos)
	{
		s.sP(pos);
		setup();
	}
	
	void setup()
	{
		s.setRadius(4);
		s.setFillColor(Color::Black);
		centerOrigin(s);
		hl.setRadius(10);
		hl.setFillColor(Color(0, 0, 0, 40));
		centerOrigin(hl);
		hl.sP(s.gP());
		controls.clear();
	}
	
	void setPosition(vecf pos, bool moveControls = false)
	{
		auto dif = pos - s.gP();
		s.sP(pos);
		hl.sP(pos);
		if (moveControls)
			for (auto& ctl : controls)
				ctl.s.move(dif);
	}

	CircleShape     s
					, hl	// highlight
	;
		// Point1 to Control1, Control1 to Control2, Control2 to Point2
	// put in one va?
	VertexArray     P1C1 {Lines}
					, C1C2 {Lines}
					, C2P2 {Lines}
	;
	bool            	isControl = false,
						isHighlighted = false;
	vector<Vert>    	controls {};
	Vert*           	parent = nullptr;
	EditorPlatform* 	parentPlat = nullptr;
	EditorGroundSeg 	seg {};
	string				surfaceType = "grass";
	Texture*			txPtr = nullptr;
	Texture*			txPtrUps = nullptr;
	bool				hasHole = false
						, hasTee = false
	;
};




struct EditorPlatform
{
	EditorPlatform () { verts.reserve(2000); }
	
	pair<Vert*, Vert*> recomputeSpline ();
	
	pair<Vert*, Vert*> updateSegs () ;

	TransformableVxArray	splVa {LineStrip};
	vector<Vert> 			verts;
	vector<Vert>			saveVerts;
	vector<vecf>			pts;
//	vector<EditorGroundSeg> segs;  //in Vert
	PlatFillInfo			fillInfo;
	string					fillTboxStr = "207 163 0 255 14 1";
	bool					isComplete = false;
};



struct Hole : public ZSprite {
		
	Hole ()
	{
		tx.loadFromFile("resources/hole.png");
		setTexture(tx);
		setOrigin(gLB().width / 2, 0);
	}
	
	
	vecf ballLoc ()
	{
		return gP() + pVec(gLB().height - 4, 90);
	}
	
	bool containsCollisionPt (const vecf& pt)
	{
		return rectWithAddedMarginOf(gGB(), 5).contains(pt);
	}
	
	bool approveVelocity (const vecf& vlc)
	{
		// make a function of approach angle and speed
		return vlc.x < 5;
	}
	
	Texture 		tx;
};



struct CourseHole
{
	
	string platformsFile;
	string holeName;
	int holeNumber;
	int par = 5;
	int strokeCt;
	Color skyColor;
};



struct Course
{
	string courseName;
	int numHoles;
	int nextHole;
	CourseHole* curHole;
	vector<CourseHole> holes;
	int par;
	int strokeCt;
	
	void reset()
	{
		nextHole = 1;
		strokeCt = 0;
	}
};



struct CollisionInfo
{
	CollisionInfo (GroundSegment* seg_ = nullptr, vecf pt = vecf(0, 0), bool cvx = false) : seg(seg_), collisionPt(pt), passingConvexEnd(cvx) {}
	GroundSegment*	seg;
	vecf			collisionPt;
	bool			passingConvexEnd = false;
};

struct CrotchInfo
{
	CrotchInfo() { }
	CrotchInfo(GroundSegment* fr, GroundSegment* co, GroundSegment* to)
		: fromSeg(fr)
		, collisionSeg(co)
		, toSeg(to)
	{ }
	
	GroundSegment* otherSeg(GroundSegment* segm) const
	{
		return (segm == fromSeg) ? collisionSeg : fromSeg;
	}
	
	GroundSegment* nextSeg() const
	{
		return collisionSeg == fromSeg->prev ? fromSeg : collisionSeg;
	}
	
	GroundSegment* prevSeg() const
	{
		return collisionSeg == fromSeg->prev ? collisionSeg : fromSeg;
	}
	
	GroundSegment*	fromSeg;
	GroundSegment*	collisionSeg;
	GroundSegment*	toSeg;
};





struct AnimFrame  //@kludgeAnim
{
	AnimFrame (const IntRect& rect, int num, string nam = "")
	: subRect(rect)
	, orderNum(num)
	, name(nam)
	{ }
	IntRect subRect;
	string name;
	int orderNum;
};


struct SurfacePhysics
{
	SurfacePhysics () {}
	SurfacePhysics (string st, float ms, float mk, float bl, float maxpow, bool putt = false)
		: surfaceType(st)
		, muS(ms)
		, muK(mk)
		, bounceLoss(bl)
		, maxPower(maxpow)
		, puttOnly(putt)
	{ }
	float 	muS;
	float 	muK;
	float 	bounceLoss;
	float	maxPower;
	string 	surfaceType;
	bool	puttOnly = false;
	// are centrifugal factors dependent on friction
};


class CourseButton : public ZSprite
{
public:
	CourseButton () { }
	CourseButton (Course& c, const vecf& pos) ;
	
	void draw (RenderTarget& target, RenderStates st) const override
	{
		target.draw(bkgdSky);
		ZSprite::draw(target, st);
		target.draw(label);
	}
	
	float			thumbWid = 400
					, thumbHt = 400
	;
	Course*	course;
	RectangleShape	bkgdSky;
	Text			label;
};


class ToolButton : public ZSprite {
public:
	
	ToolButton () {}
	ToolButton (const Texture& tx) {
		
		s.setTexture(tx);
		highlight.setSize(vecf(s.gLB().width, min(s.gLB().height, 32.f)));
		highlight.setFillColor(Color(0, 0, 0, 60));
	}

	
	RectangleShape		highlight;
	bool				isSelected = false;
	string				key;
	vecf 				posDifFromToolbar;
	bool				isFill = false;
};


class ToolWindow : public Drawable
{
public:
	
	ToolWindow (const vecf& size)
	{
		totalRect.setOutlineThickness(2);
		totalRect.setOutlineColor(Color(DKORANGE));
		totalRect.setFillColor(Color(255, 127, 0, 100));
		totalRect.setSize(size);
	}
	
	void init (const Font& font)
	{
		selectButton.setSize(vecf(totalRect.getSize().x - 6, 20));
		selectButton.setOutlineColor(DKORANGE75);
		selectButton.setOutlineThickness(2);
		selectButton.setFillColor(addRed(Color::Yellow, 35));
	
		holeButton.setSize(vecf(totalRect.getSize().x - 6, 20));
		holeButton.setOutlineColor(DKORANGE75);
		holeButton.setOutlineThickness(2);
		holeButton.setFillColor(decreaseSaturation(Color::Blue, 45));
		
		teeButton.setSize(vecf(totalRect.getSize().x - 6, 20));
		teeButton.setOutlineColor(DKORANGE75);
		teeButton.setOutlineThickness(2);
		teeButton.setFillColor(decreaseSaturation(addRed(Color::Green, 35), 35));
		
		highlight.setSize(selectButton.getSize() + vecf(10, 10));
		highlight.setFillColor(Color(0, 0, 0, 40));
		centerOrigin(highlight);
		
		Text* txts[3] = {&selectTxt, &holeTxt, &teeTxt};
		string labels[3] = {"Select", "Hole", "Tee"};
		forNum(3) {
			Text& txt = *(txts[i]);
			txt = Text(labels[i], font, 16);
			txt.setFillColor(CHARCOAL);
			txt.setOutlineThickness(1);
			txt.setOutlineColor(Color::White);
			centerOrigin(txt);
		}
		move({0, 0});
	}
	
	void draw (RenderTarget& rt, RenderStates states) const
	{
		rt.draw(totalRect);
		rt.draw(selectButton);
		rt.draw(holeButton);
		rt.draw(teeButton);
		rt.draw(selectTxt);
		rt.draw(holeTxt);
		rt.draw(teeTxt);
		rt.draw(highlight);
		
		for (auto& tb : toolButtons) {
			rt.draw(tb.second);
			if (tb.second.isSelected)
				rt.draw(tb.second.highlight);
		}
	}
	
	void move (const vecf& moveDif)
	{
		totalRect.move(moveDif);
		selectButton.sP(totalRect.gP() + vecf(3, 20));
		holeButton.sP(selectButton.gP() + vecf(0, 30));
		teeButton.sP(holeButton.gP() + vecf(0, 30));
		if (highlight.gP().x >= 0) // temporary
			highlight.move(moveDif);
		Text* txts[3] = {&selectTxt, &holeTxt, &teeTxt};
		RectangleShape* parentRects[3] = {&selectButton, &holeButton, &teeButton};
		forNum(3)
		txts[i]->sP(rectCenter(*(parentRects[i])) + vecf(0, -5));
		for (auto& btn : toolButtons) {
			btn.second.sP(totalRect.gP() + btn.second.posDifFromToolbar);
			btn.second.highlight.sP(btn.second.gP());
		}
	}
	
	RectangleShape				totalRect;
	RectangleShape				selectButton
								, holeButton
								, teeButton
								, highlight
	;
	Text						selectTxt
								, holeTxt
								, teeTxt
	;
	map<string, ToolButton>			toolButtons;
	
	float			spacing = 12;
	bool			isActive = true
					, clickDragging = false
	;
	
};


#endif /* objects_hpp */
