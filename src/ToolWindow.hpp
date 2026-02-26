//
//  ToolWindow.hpp
//  ZGolf
//
//  Created by John Ziegler on 2/24/26.
//  Copyright © 2026 John Ziegler. All rights reserved.
//

#ifndef ToolWindow_hpp
#define ToolWindow_hpp

#include "zgolf.hpp"


class ToolButton : public ZSprite
{
public:
	ToolButton () { }
	
	ToolButton (const Texture& tx)
	{
		s.setTexture(tx);
		highlight.setSize(vecf(s.gLB().width, min(s.gLB().height, 32.f)));
		highlight.setFillColor(Color(0, 0, 0, 60));
	}
	
	RectangleShape		highlight;
	string				key;
	vecf 				posDifFromToolbar;
	bool				isSelected = false;
	bool				isFill = false;
};



class ToolWindow : public Drawable
{
public:
	ToolWindow (const vecf& size);

	void init ();
	
	void draw (RenderTarget& rt, RenderStates states) const;
	
	void move (const vecf& moveDif);
	
	RectangleShape				totalRect
								, selectButton
								, holeButton
								, teeButton
								, highlight
	;
	Text						selectTxt
								, holeTxt
								, teeTxt
	;
	map<string, ToolButton>		toolButtons;
	
	float						spacing = 12;
	bool						isActive = true
								, clickDragging = false
	;
};

#endif /* ToolWindow_hpp */
