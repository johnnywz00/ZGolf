//
//  platforms.cpp
//  ZGolf
//
//  Created by John Ziegler on 2/24/26.
//  Copyright © 2026 John Ziegler. All rights reserved.
//

#include "zgolf.hpp"

#define vwWid State::viewWid()
#define vwHt State::viewHt()

pair<Vert*, Vert*> EditorPlatform::recomputeSpline ()
{
	splVa.clear();
	auto sz = verts.size();
	// no spline to draw if only one or zero points created
	if (sz < 2)
		return {nullptr, nullptr};
	
	for (int i = 1; i <= sz; ++i) {
		Vert& v = verts[i - 1];
		v.P1C1.clear();
		v.C1C2.clear();
		v.C2P2.clear();
		auto pos1 = v.s.gP();
		// [i % sz] so that final vert will connect to first vert
		auto pos2 = verts[i % sz].s.gP();
		if (i == 1)
			splVa.append(Vertex(pos1, Color::Black));
		
		// only draw spline from final vert to initial vert if a control point has been added on the final one, to signify completing the circuit
		if (v.controls.size() == 0) {
			if (isComplete || i != sz)
				splVa.append(Vertex(pos2, Color::Black));
			continue;
		}
		else if (v.controls.size() == 1) {   // one control point: quadratic curve
			vecf c1 = v.controls[0].s.gP();
			vecf p1c1 = c1 - pos1;
			vecf c2p2 = pos2 - c1;
			vecf inc1 = p1c1 * .01f;
			vecf inc3 = c2p2 * .01f;
			for (float j = 1; j <= 100; ++j) {
				if (j == 100) {
					splVa.append(Vertex(pos2, Color::Black));
					break;
				}
				vecf pt1 = pos1 + inc1 * j;
				vecf pt3 = c1 + inc3 * j;
				vecf dif1 = pt3 - pt1;
				vecf incd1 = dif1 * .01f;
				vecf pt6 = pt1 + vecf(incd1.x * j, incd1.y * j);
				
				splVa.append(Vertex(pt6, Color::Black));
			}
		}
		else if (v.controls.size() == 2) { // two control points: cubic curve
			vecf c1 = v.controls[0].s.gP();
			vecf c2 = v.controls[1].s.gP();
			vecf p1c1 = c1 - pos1;
			vecf c1c2 = c2 - c1;
			vecf c2p2 = pos2 - c2;
			vecf inc1 = p1c1 * .01f;
			vecf inc2 = c1c2 * .01f;
			vecf inc3 = c2p2 * .01f;
			for (float j = 1; j <= 100; ++j) {
				if (j == 100) {
					splVa.append(Vertex(pos2, Color::Black));
					break;
				}
				vecf pt1 = pos1 + inc1 * j;
				vecf pt2 = c1 + inc2 * j;
				vecf pt3 = c2 + inc3 * j;
				vecf dif1 = pt2 - pt1;
				vecf dif2 = pt3 - pt2;
				vecf incd1 = dif1 * .01f;
				vecf incd2 = dif2 * .01f;
				vecf pt4 = pt1 + incd1 * j;
				vecf pt5 = pt2 + incd2 * j;
				vecf dif3 = pt5 - pt4;
				vecf pt6 = pt4 + vecf(dif3.x * j * .01, dif3.y * j * .01);
				
				splVa.append(Vertex(pt6, Color::Black));
			}
		}
		// draw guidelines
		Color c = Color(230, 230, 230, 200);
		vecf c1 = v.controls[0].s.gP();
		vecf c2;
		if (v.controls.size() == 2)
			c2 = v.controls[1].s.gP();
		else c2 = c1;
		Color col = Color(200, 200, 255);
		v.C1C2.append(VXC(c1.x, c1.y, col));
		v.C1C2.append(VXC(c2.x, c2.y, col));
		auto dif1 = c1 - pos1;
		float m1;
		if (dif1.x == 0) {
			v.P1C1.append(VXC(c1.x, 0, c));
			v.P1C1.append(VXC(c1.x, vwHt, c));
		}
		else {
			m1 = dif1.y / dif1.x;
			v.P1C1.append(VXC(0, c1.y - m1 * c1.x, c));
			v.P1C1.append(VXC(vwWid, m1 * vwWid + (c1.y - m1 * c1.x), c));
		}
		dif1 = pos2 - c2;
		if (dif1.x == 0) {
			v.C2P2.append(VXC(c2.x, 0, c));
			v.C2P2.append(VXC(c2.x, vwHt, c));
		}
		else {
			m1 = dif1.y / dif1.x;
			v.C2P2.append(VXC(0, c2.y - m1 * c2.x, c));
			v.C2P2.append(VXC(vwWid, m1 * vwWid + (c2.y - m1 * c2.x), c));
		}
	}
	return updateSegs();
}

pair<Vert*, Vert*> EditorPlatform::updateSegs ()
{
	pair<Vert*, Vert*> holeAndTee {nullptr, nullptr};
	int sz = (int)verts.size();
	forNum(sz) {
		Vert& vert = verts[i];
		/* Don't connect the verts cyclically until "finishGround" has been
		 * called on this platform, since during initial platform spline creation
		 * the ground segment connecting last to first would just be a nuisance.
		 */
		int idx = !isComplete ? max(0, i - 1) : (i + sz - 1) % sz;
		auto start = verts[idx].s.gP();
		auto end = vert.s.gP();
		EditorGroundSeg g {start, end};
		Texture* txPtr;
		if (!g.facesUp && vert.txPtrUps != nullptr) {
			// If doing up/downfacing schemes, the downfacing surfaceType
			// will need to be stored and read from
			g.surfaceType = "dirt";
			txPtr = vert.txPtrUps;
		}
		else {
			g.surfaceType = vert.surfaceType;
			txPtr = vert.txPtr;
		}
		g.initSprite(*txPtr);
		if (vert.hasHole) {
			g.hasHole = true;
			holeAndTee.first = &vert;
		}
		if (vert.hasTee) {
			g.hasTee = true;
			holeAndTee.second = &vert;
		}
		vert.seg = g;
	}
	return holeAndTee;
}

