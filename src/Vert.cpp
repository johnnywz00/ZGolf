//
//  Vert.cpp
//  ZGolf
//
//  Created by John Ziegler on 2/24/26.
//  Copyright © 2026 John Ziegler. All rights reserved.
//

#include "zgolf.hpp"

void Vert::setup()
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

void Vert::setPosition(vecf pos, bool moveControls)
{
	auto dif = pos - s.gP();
	s.sP(pos);
	hl.sP(pos);
	if (moveControls)
		for (auto& ctl : controls)
			ctl.s.move(dif);
}

