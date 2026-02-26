//
//  Vert.hpp
//  ZGolf
//
//  Created by John Ziegler on 2/24/26.
//  Copyright © 2026 John Ziegler. All rights reserved.
//

#ifndef Vert_hpp
#define Vert_hpp

#include "zgolf.hpp"


class EditorPlatform;

class Vert
{
public:
	Vert() { setup(); }
	
	Vert(const vecF& pos)
	{
		s.sP(pos);
		setup();
	}
	
	void setup();

	void setPosition(vecf pos, bool moveControls = false);

	CircleShape     s
					, hl	// highlight
	;
	// Point1 to Control1, Control1 to Control2, Control2 to Point2
	// put in one va?
	VertexArray     P1C1 {Lines}
					, C1C2 {Lines}
					, C2P2 {Lines}
	;
	EditorGroundSeg 	seg;
	vector<Vert>    	controls;
	Vert*           	parent = nullptr;
	EditorPlatform* 	parentPlat = nullptr;
	Texture*			txPtr = nullptr;
	Texture*			txPtrUps = nullptr;
	string				surfaceType = "grass";
	bool            	isControl = false
						, isHighlighted = false
						, hasHole = false
						, hasTee = false
	;
};

#endif /* Vert_hpp */
