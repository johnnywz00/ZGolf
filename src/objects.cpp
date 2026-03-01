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
	
	setTexture(gTexture(c.holes[0].platformsFile));
	setOrigin(gLB().width / 2, gLB().height);
	auto factor = thumbWid / gLB().width;
	setScale(factor, factor);
	setPosition(pos);
	
	bkgdSky.setSize({gGB().width, gGB().height});
	bkgdSky.setFillColor(SKYBLUE);
	bkgdSky.setOrigin(gGB().width / 2, gGB().height);
	bkgdSky.setPosition(pos);
	
	label.setFont(gFont("menuTitle"));
	label.setCharacterSize(32);
	label.setOutlineColor(DKORANGE75);
	label.setOutlineThickness(3);
	label.setFillColor(BUTTERSKY);
	label.setString(c.courseName);
	::centerOrigin(label);
	label.setPosition(pos + pVec(30, 90));
}
