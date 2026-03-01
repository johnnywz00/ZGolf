//
//  objects.hpp
//  ZGolf
//
//  Created by John Ziegler on 1/9/25.
//  Copyright © 2025 John Ziegler. All rights reserved.
//

#ifndef objects_hpp
#define objects_hpp

#include "zgolf.hpp"

struct Hole : public ZSprite
{
	Hole ()
	{
		setTexture(gTexture("hole"));
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
};



struct CourseHole
{
	Color 		skyColor;
	string 		platformsFile;
	string 		holeName;
	vecF		viewSize {1728, 1117};
	int 		holeNumber;
	int 		par = 5;
	int 		strokeCt;
};



struct Course
{
	void reset()
	{
		nextHole = 1;
		strokeCt = 0;
	}

	vector<CourseHole> 	holes;
	CourseHole* 		curHole;
	string 				courseName;
	int 				numHoles;
	int 				nextHole;
	int 				par;
	int 				strokeCt;
};



struct CollisionInfo
{
	CollisionInfo (GroundSegment* seg_ = nullptr, vecf pt = vecf(0, 0), bool cvx = false)
		: seg(seg_)
		, collisionPt(pt)
		, passingConvexEnd(cvx)
	{ }
	
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
	
	GroundSegment*		fromSeg;
	GroundSegment*		collisionSeg;
	GroundSegment*		toSeg;
};



struct AnimFrame  //@kludgeAnim
{
	AnimFrame (const IntRect& rect, int num, string nam = "")
		: subRect(rect)
		, orderNum(num)
		, name(nam)
	{ }
	
	IntRect 	subRect;
	string 		name;
	int 		orderNum;
};


struct SurfacePhysics
{
	SurfacePhysics () { }
	
	SurfacePhysics (string st, float ms, float mk, float bl, float maxpow, bool putt = false)
		: surfaceType(st)
		, muS(ms)
		, muK(mk)
		, bounceLoss(bl)
		, maxPower(maxpow)
		, puttOnly(putt)
	{ }
	
	string 		surfaceType;
	float 		muS;
	float 		muK;
	float 		bounceLoss;
	float		maxPower;
	bool		puttOnly = false;
	// are centrifugal factors dependent on friction
};


class CourseButton : public ZSprite
{
public:
	CourseButton () { }
	
	CourseButton (Course& c, const vecf& pos);
	
	void draw (RenderTarget& target, RenderStates st) const override
	{
		target.draw(bkgdSky);
		ZSprite::draw(target, st);
		target.draw(label);
	}
	
	RectangleShape	bkgdSky;
	Text			label;
	Course*			course;
	float			thumbWid = 400
					, thumbHt = 400
	;
};

#endif /* objects_hpp */
