//
//  GroundSegment.cpp
//  ZGolf
//
//  Created by John Ziegler on 2/24/26.
//  Copyright © 2026 John Ziegler. All rights reserved.
//

#include "zgolf.hpp"

GroundSegment::GroundSegment (const vecF& s, const vecF& e, float ballRadius)
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

void GroundSegment::initSprite (const Texture& tx)
{
	spr = Sprite(tx);
	spr.setTextureRect(IntRect(
							   0, 0, length,
							   max( int(spr.getTexture()->getSize().y), 7)));
	spr.setOrigin(0, spr.gLB().height - 4.5); // 4.5 is ballRadius
	spr.sP(start);
	spr.sRot(angle);
}

void GroundSegment::setConcavityToNeighbors ()
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
