//
//  objects.cpp
//  ZGolf
//
//  Created by John Ziegler on 12/13/25.
//  Copyright © 2025 John Ziegler. All rights reserved.
//

#include "zgolf.hpp"

CourseButton::CourseButton (Course& c, const vecf& pos)
{
	course = &c;
	
	bkgdSky.setSize({thumbWid, thumbHt});
	bkgdSky.setFillColor(SKYBLUE);
	::centerOrigin(bkgdSky);
	bkgdSky.setPosition(pos);
	
	setTexture(gTexture(c.holes[0].platformsFile));
	centerOrigin();
	setScale(thumbWid / gLB().width, thumbHt / gLB().height);
	setPosition(pos);
	
	label.setFont(gFont("menuTitle"));
	label.setCharacterSize(32);
	label.setOutlineColor(DKORANGE75);
	label.setOutlineThickness(3);
	label.setFillColor(BUTTERSKY);
	label.setString(c.courseName);
	::centerOrigin(label);
	label.setPosition(pos + pVec(thumbHt / 2 + 30, 90));
}
