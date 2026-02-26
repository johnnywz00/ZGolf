//
//  GroundSegment.hpp
//  ZGolf
//
//  Created by John Ziegler on 2/24/26.
//  Copyright © 2026 John Ziegler. All rights reserved.
//

#ifndef GroundSegment_hpp
#define GroundSegment_hpp

#include "zgolf.hpp"


class GroundSegment
{
public:
	GroundSegment (const vecF& s, const vecF& e, float ballRadius = 4.5);
	
	virtual void initSprite (const Texture& tx);
	
	void setConcavityToNeighbors ();
	
	bool ptIsNormalFromCheckline(const vecf& pt)
	{
		vecf pdif = toPolar(pt - collisionCheckLine.pt1);
		return clockwiseOf(pdif.y, angle);
	}
	
	Sprite 			spr;
	Text 			txt;
	LineSegment 	lseg;
	LineSegment		collisionCheckLine;
	FloatRect 		bounds;
	GroundSegment*	prev = nullptr;
	GroundSegment*	next = nullptr;
	string			surfaceType = "grass";
	vecF 			start
					, end
					, mid
					, checkLineIsctPrev
					, checkLineIsctNext
	;
	float 			angle
					, oppAngle
					, normal
					, oppNormal
					, length
					, slope
					, yIcpt
					, minx
					, maxx
					, miny
					, maxy
					, xIcpt
					, muK = ::muK  //.01;
					, muS = ::muS  //.1;
					, bounceLoss = ::bounceLoss  // used as mag * (1 - bounceLoss)
	;
	int 			id = 0;
	bool 			isVertical = false
					, isHorizontal = false
					, concaveFromPrev = false	// set by platform loading
					, concaveToNext = false
					, facesUp = false
	;
	
	// DEBUG
	VertexArray     n {Lines};
};



struct EditorGroundSeg : public GroundSegment
{
	EditorGroundSeg () : GroundSegment(vecf(0,0), vecf(0,1)) { }
	
	EditorGroundSeg (const vecF& s, const vecF& e) : GroundSegment(s, e) {	}
	
	void initSprite (const Texture& tx) override
	{
		GroundSegment::initSprite(tx);
		spr.setColor(Color(255, 255, 255, 120));
	}
	
	string 	surfaceType;
	bool 	hasHole = false;
	bool 	hasTee = false;
};

#endif /* GroundSegment_hpp */
