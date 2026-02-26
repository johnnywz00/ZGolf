//
//  ToolWindow.cpp
//  ZGolf
//
//  Created by John Ziegler on 2/24/26.
//  Copyright © 2026 John Ziegler. All rights reserved.
//

#include "zgolf.hpp"

ToolWindow::ToolWindow (const vecf& size)
{
	totalRect.setOutlineThickness(2);
	totalRect.setOutlineColor(Color(DKORANGE));
	totalRect.setFillColor(Color(255, 127, 0, 100));
	totalRect.setSize(size);
}

void ToolWindow::init ()
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
		txt = Text(labels[i], gFont("toolButton"), 16);
		txt.setFillColor(CHARCOAL);
		txt.setOutlineThickness(1);
		txt.setOutlineColor(Color::White);
		centerOrigin(txt);
	}
	move({0, 0});
}

void ToolWindow::draw (RenderTarget& rt, RenderStates states) const
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

void ToolWindow::move (const vecf& moveDif)
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
