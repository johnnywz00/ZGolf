//
//  designMethods.cpp
//  ZGolf
//
//  Created by John Ziegler on 11/2/25.
//  Copyright © 2025 John Ziegler. All rights reserved.
//

#include "zgolf.hpp"
#include "sfmlApp.hpp"

bool State::handleTextEvent (Event& event)
{
	if (activeTbox
		&& (event.type == Event::TextEntered
			|| event.type == Event::KeyPressed
			|| event.type == Event::KeyReleased)) {
		if (event.type == Event::TextEntered) {
			if (event.text.unicode == 8) {
				if (isShiftPressed())
					activeTbox->clear();
				else activeTbox->deleteLastChar();
			}
			else if (event.text.unicode == 9) ; // Don't write the \t
			else activeTbox->appendText(event.text.unicode);
		}
		if (event.type == Event::KeyPressed
			&& (event.key.code == Keyboard::Escape || event.key.code == Keyboard::Enter)) {
			if (activeTbox == &fillInfoTbox) {
				curPlat->fillTboxStr = fillInfoTbox.boxTxt.getString();
			}
			deactivateCurTbox();
		}
		return true;
	}
	return false;
}

void State::switchToDesign ()
{
	mode = design;
	ballActive = false;
	
	ball.sP(-100, -100);
	hole.sP(-100, -100);
	
	holeWhenEnteringMode = curCourse->curHole;
	loadPlatformData(curPlatFile);
	
	curSurfType = "grass";
	toolWin.toolButtons["grass"].isSelected = true;
	activateSelectButton();
	app->setRedrawColor(Color(254, 252, 250));
	restoreView();
}

void State::loadToolbarButtons()
{
	float tbY = toolWin.teeButton.getPosition().y + toolWin.teeButton.getSize().y + toolWin.spacing;
	forNum (surfaceTypeList.size()) {
		auto key = surfaceTypeList[i].second;
		ToolButton tb {gTexture(key)};
		tb.sP(toolWin.totalRect.gP().x + 3,
			  tbY);
		tbY += tb.gLB().height + toolWin.spacing;
		tb.highlight.sP(tb.gP());
		tb.posDifFromToolbar = tb.gP() - toolWin.totalRect.gP();
		tb.key = key;
		toolWin.toolButtons.insert({key, tb});
	}
	tbY += 40;
	
	forNum (fillTypeList.size()) {
		auto key = fillTypeList[i].second;
		ToolButton tb {gTexture(key)};
		tb.setTextureRect(IntRect(0, 0, 64, 64));
		tb.isFill = true;
		tb.sP(toolWin.totalRect.gP().x + 3,
			  tbY);
		tbY += tb.gLB().height + toolWin.spacing;
		tb.posDifFromToolbar = tb.gP() - toolWin.totalRect.gP();
		tb.key = key;
		toolWin.toolButtons.insert({key, tb});
	}
	toolWin.totalRect.setSize({toolWin.totalRect.getSize().x, tbY - toolWin.totalRect.getPosition().y});
	toolWin.move({30, 30}); // Initializes button positions
}

void State::loadPlatformData (string fname)
{
	curPlatforms.clear();
	ifstream fs {resourcePath() / "levels" / (fname + ".txt")};
	string line;
	EditorPlatform ep;
	
	while (getline(fs, line)) {
		if (line.empty()
			|| line[0] == '#'
			|| line == "SPRITE_CACHED")
			continue;
		if (line[0] == ':') {
			//			PlatFillInfo pfi;
			stringstream ss {line};
			string tok;
			ss >> tok; // Pass over colon
			if (ss >> tok) {
				if (tok == "colordev") {
					ep.fillTboxStr = line.substr(line.find('v') + 2);
					/*
					 pfi.type = PlatFillInfo::FillType::colorDev;
					 uint red, green, blue, alpha;
					 int dev, reps;
					 ss >> red >> green >> blue >> alpha >> dev >> reps;
					 PlatFillInfo::ColorDevInfo cdi {
					 Color(red, green, blue, alpha), dev, reps
					 };
					 pfi.arg = cdi;
					 */
				}
				else if (tok == "imagepixs") {
					ep.fillTboxStr = line.substr(line.find('s') + 2);
					/*
					 pfi.type = PlatFillInfo::FillType::imagePixs;
					 ss >> tok;
					 pfi.arg = tok;
					 */
				}
			}
			//			ep.fillInfo = pfi;
			
			ep.isComplete = true;
			curPlatforms.push_back(ep);
			for (auto& v : curPlatforms.back().verts) {
				v.parentPlat = &curPlatforms.back();
			}
			finishGround(&curPlatforms.back(), false);
			
			ep = EditorPlatform();
			continue;
		}
		stringstream ss {line};
		string tok, skip;
		Vert v;
		vecf pos;
		auto peek = ss.peek();
		if (peek == 'T' || peek == 'H') {
			ss >> tok;
			if (tok == 'T')
				v.hasTee = true;
			else if (tok == 'H')
				v.hasHole = true;
		}
		//		ss >> pos.x >> pos.y >> skip >> skip;
		ss >> skip >> skip >> pos.x >> pos.y;
		v.setPosition(pos);
		if (ss >> tok) {
			v.surfaceType = tok;
			v.txPtr= &(gTexture(tok));
		}
		else {
			//potentially read scheme for upfacing/downfacing segs
			/* Defaults */
			v.txPtr = &(gTexture("grass"));
			// This shouldn't work unless v.surfaceType is dynamic based on
			// updateSegs -> facesUp?
			v.txPtrUps = &(gTexture("dirt"));
		}
		ep.verts.push_back(v);
	}
	
	fs.close();
	filenameTbox.setText(fname);
	
	newPlatform();
}

void State::designKeyPress (Keyboard::Key k)
{
	switch(k) {
			
		case Keyboard::X:
			clearMap();
			break;
		
		case Keyboard::U:
			finishGround(curPlat, !curPlat->isComplete);
			break;
		
		case Keyboard::J:
			saveHole();
			break;
		
		case Keyboard::Backspace:
			maybeEraseSelectedVert();
			break;
		
		case Keyboard::Space:
			if (curTool != "select") {
				activateSelectButton();
				break;
			}
			/* Else fall through to Enter v v */
		case Keyboard::Enter:
			if (gHighlighted) {
				gHighlighted->isHighlighted = false;
				gHighlighted = nullptr;
			}
			break;
			
		case Keyboard::Slash:
			showInstr = !showInstr;
			break;
			
		default:
			break;
	}
}

void State::designClick (int x, int y)
{
	if (showInstr) {
		showInstr = false;
		return;
	}
	
	bool clickedTool = false;
	auto clickTool = [&](string str, RectangleShape& but) {
		curTool = str;
		toolWin.highlight.sP(rectCenter(but));
		clickedTool = true;
	};
	
	if (toolWin.selectButton.gGB().contains(x, y))
		clickTool("select", toolWin.selectButton);
	else if (toolWin.holeButton.gGB().contains(x, y))
		clickTool("placeHole", toolWin.holeButton);
	else if (toolWin.teeButton.gGB().contains(x, y))
		clickTool("placeTee", toolWin.teeButton);
	
	for (auto& tb : toolWin.toolButtons) {
		if (tb.second.gGB().contains(x, y)) {
			if (tb.second.isFill) {
				curPlat->fillTboxStr = tb.second.key;
				fillInfoTbox.boxTxt.setString(tb.second.key);
			}
			else {
				toolWin.toolButtons[curSurfType].isSelected = false;
				curTool = "setSurface";
				curSurfType = tb.second.key;
				tb.second.isSelected = true;
				toolWin.highlight.sP(-200, -200); //temporary
			}
			clickedTool = true;
			break;
		}
	}
	
	if (!clickedTool) {
		if (toolWin.totalRect.gGB().contains(x, y)) {
			toolWin.clickDragging = true;
		}
		//		else if (curTool == "someTool") { }
		else {
			
			if (filenameTbox.tbox.gGB().contains(x, y)) {
				deactivateCurTbox();
				filenameTbox.setActive(true);
				activeTbox = &filenameTbox;
				return;
			}
			else if (fillInfoTbox.tbox.gGB().contains(x, y)) {
				deactivateCurTbox();
				fillInfoTbox.setActive(true);
				activeTbox = &fillInfoTbox;
				return;
			}
			else if (activeTbox) {
				deactivateCurTbox();
				return;
			}
			
			for (auto& p : curPlatforms) {
				auto sz = p.verts.size();
				for (int i = 0; i < sz; ++i) {
					Vert& vert = p.verts[i];
					bool segsNeedUpdate = false;
					
					/* Click on existing Vert */
					if (vert.s.gGB().contains(x, y)) {
						if (curTool == "placeHole") {
							for (auto& plat : curPlatforms)
								for (auto& v : plat.verts) {// inefficient: store
									v.hasHole = false;
									v.seg.hasHole = false;
								}
							vert.hasHole = true;
							vert.seg.hasHole = true;
							segsNeedUpdate = true;
						}
						else if (curTool == "placeTee") {
							for (auto& plat : curPlatforms)
								for (auto& v : plat.verts) {// inefficient: store
									v.hasTee = false;
									v.seg.hasTee = false;
								}
							vert.hasTee = true;
							vert.seg.hasTee = true;
							segsNeedUpdate = true;
						}
						else if (curTool == "setSurface") {
							auto func = [&](Vert& cur) {
								cur.surfaceType = curSurfType;
								cur.seg.surfaceType = curSurfType;
								cur.txPtr = &(gTexture(curSurfType));
							};
							if (isCmdPressed())
								for (auto& v : p.verts)
									func(v);
							else func(vert);
							segsNeedUpdate = true;
						}
						else if (curTool == "select") {
							gClicked = &vert;
							if (isShiftPressed()) {
								if (gHighlighted) {
									bool wasHighlighted = gHighlighted == gClicked;
									gHighlighted->isHighlighted = false;
									if (wasHighlighted) {
										gHighlighted = nullptr;
										return;
									}
								}
								gHighlighted = gClicked;
								gHighlighted->isHighlighted = true;
							}
						}
						
						setCurPlat(&p);
						if (segsNeedUpdate) {
							auto htSegs = p.updateSegs();
							updateHoleAndTee(htSegs);
						}
						return;
					}
					
					/* CHECK THIS IF ADDING NEW TOOLS OR EDIT TYPES */
					if (curTool != "select")
						continue;
					
					/* Click on existing control point */
					for (auto& c : vert.controls) {
						if (c.s.gGB().contains(x, y)) {
							setCurPlat(&p);
							if (isShiftPressed()) {
								vert.controls.erase(p.verts[i].controls.begin() + indexOfRef(p.verts[i].controls, c));
								p.recomputeSpline();
							}
							else gClicked = &c;
							return;
						}
					}
				}
			} // end for plat
			
			/* CHECK THIS IF ADDING NEW TOOLS OR EDIT TYPES */
			if (curTool != "select")
				return;
			
			/* New point if not clicking on existing */
			Vert v {vecf(x, y)};
			if (curPlat->isComplete && !gHighlighted)
				setCurPlat(&curPlatforms.back());
			v.parentPlat = curPlat;
			if (curPlat->verts.size())
				v.seg = EditorGroundSeg(curPlat->verts.back().s.gP(), vecf(x, y));
			v.seg.surfaceType = curSurfType;
			v.surfaceType = curSurfType; //which one to use ^^
			v.txPtr = &(gTexture(curSurfType));
			// if scheme, v.txPtrUps = scheme.ups
			
			if (gHighlighted) {
				if (isShiftPressed() && gHighlighted->controls.size() < 2) {
					v.isControl = true;
					v.parent = gHighlighted;
					v.s.setFillColor(Color::Green);
					gHighlighted->controls.push_back(v);
				}
				else if (!isShiftPressed())
					curPlat->verts.insert(curPlat->verts.begin() + indexOfRef(curPlat->verts, *gHighlighted) + 1, v);
			}
			else if (!isShiftPressed())
				curPlat->verts.push_back(v);
			
			updateHoleAndTee(curPlat->recomputeSpline());
		}
	}
}

void State::designUpdate ()
{
	auto floatMouse = toVecF(mouseVec);
	auto mouseDif = floatMouse - toVecF(oldMouse);
	if (toolWin.clickDragging) {
		toolWin.move(mouseDif);
	}
	
	if (gClicked)  {
		auto plat = gClicked->parentPlat;
		if (isCmdPressed()) {
			vecf oldPos = gClicked->s.gP();
			vecf dif = floatMouse - oldPos;
			for (auto& v : plat->verts) {
				v.s.move(dif);
				v.hl.move(dif);
				for (auto& c : v.controls)
					c.s.move(dif);
			}
		}
		gClicked->setPosition(floatMouse);
		
		plat->recomputeSpline();
	}
	
	string str = curTool + "   ct: " + tS(curPlatforms.size()) + "    cur: ";
	string curStr = "NULL";
	forNum(curPlatforms.size()) {
		if (&curPlatforms[i] == curPlat)
			curStr = tS(i);
	}
	statsTxt.setString(str + curStr);
}

void State::designDraw ()
{
	auto w = rwin;
	{ // /////////////  PLAYING WITH GRADIENT SPLINE
//		VertexArray va {TriangleStrip};
//		float halfWid = 8;
//		Color c1 = DKORANGE50;
//		Color c2 = ORANGE;
//		
//		for (auto& p : curPlatforms) {
//			if (p.splVa.getVertexCount() < 2)
//				continue;
//			drt.clear(Color::Transparent);
//			va.clear();
//			for (int i = 0; i < p.splVa.getVertexCount() - 1; ++i) {
//				auto pos1 = p.splVa[i].position;
//				auto pos2 = p.splVa[i + 1].position;
//				auto pdif = toPolar(pos2 - pos1);
//				va.appendPtC(pos1 + pVec(halfWid, pdif.y + 90), c1);
//				va.appendPtC(pos1 + pVec(halfWid, pdif.y + 270), c2);
//				va.appendPtC(pos2 + pVec(halfWid, pdif.y + 90), c1);
//				va.appendPtC(pos2 + pVec(halfWid, pdif.y + 270), c2);
//				
//			}
//			drtDraw(va);
//		}
//		w->draw(rts);
	}
	
	for (auto& p : curPlatforms) {
		for (auto& v : p.verts) {
			w->draw(v.seg.spr);
			if (&p == curPlat) {
				w->draw(v.P1C1);
				w->draw(v.C1C2);
				w->draw(v.C2P2);
				for (auto& c : v.controls) {
					w->draw(c.s);
				}
			}
			w->draw(v.s);
			if (v.isHighlighted)
				w->draw(v.hl);
		}
		w->draw(p.splVa);
	}
	w->draw(hole);
	w->draw(ball);
	
	w->draw(filenameTbox);
	w->draw(fillInfoTbox);
	w->draw(instrLabel);
	w->draw(toolWin);
	if (showInstr) {
		w->draw(instrRect);
		w->draw(instrTxt);
	}
	
//	w->draw(mouseTxt);
	w->draw(statsTxt);
	
	w->draw(rts); // ///
}

bool State::finishGround (EditorPlatform* plat, bool makeNew)
{
	plat->pts.clear();
	plat->saveVerts.clear();
	auto sz = plat->verts.size();
	if (sz < 2)
		return false;
	
	for (int i = 1; i <= sz; ++i) {
		Vert& v = plat->verts[i - 1];
		Vert& v2 = plat->verts[i % sz];
		auto pos1 = v.s.gP();
		auto pos2 = v2.s.gP();
		if (i == 1) {
			Vert sv {v};
			sv.setPosition(pos1);
			plat->saveVerts.push_back(sv);
			plat->pts.push_back(pos1);
		}
		if (v.controls.size() == 0) {
			Vert sv {v2};
			sv.setPosition(pos2);
			plat->saveVerts.push_back(sv);
			plat->pts.push_back(pos2);
			continue;
		}
		/* Quadratic curve */
		else if (v.controls.size() == 1) {
			// store the control point position
			vecf c1 = v.controls[0].s.gP();
			// vector from pt1 to control
			vecf p1c1 = c1 - pos1;
			// vector from control to pt2
			vecf c2p2 = pos2 - c1;
			vecf inc1 = p1c1 * .01f;
			vecf inc3 = c2p2 * .01f;
			float ct = 0;
			vecf lastPt = pos1;
			for (float j = 1; j <= 100; ++j) {
				if (j == 100) {
					Vert sv {v2};
					sv.setPosition(pos2);
					plat->saveVerts.push_back(sv);
					plat->pts.push_back(pos2);
					break;
				}
				vecf pt1 = pos1 + inc1 * j;
				vecf pt3 = c1 + inc3 * j;
				vecf dif1 = pt3 - pt1;
				vecf incd1 = dif1 * .01f;
				vecf pt6 = pt1 + vecf(incd1.x * j, incd1.y * j);
				
				float h = hyp(pos2, pt6);
				/* Don't want to end with a piece smaller than minseg,
				 * so the last segment is likely to be a little longer
				 * than the others.
				 */
				if (h < minseg * 2) {
					Vert sv {v2};
					sv.setPosition(pos2);
					plat->saveVerts.push_back(sv);
					plat->pts.push_back(pos2);
					break;
				}
				h = hyp(lastPt, pt6);
				if (h >= avgSeg) {
					Vert sv {v2};
					sv.setPosition(pt6);
					sv.controls.clear();
					plat->saveVerts.push_back(sv);
					plat->pts.push_back(pt6);
				}
				else {
					ct += h;
					if (ct >= avgSeg) {
						Vert sv {v2};
						sv.setPosition(pt6);
						sv.controls.clear();
						plat->saveVerts.push_back(sv);
						plat->pts.push_back(pt6);
						ct = 0;
					}
				}
				lastPt = pt6;
			}
		}
		
		/* Cubic curve */
		else if (v.controls.size() == 2) {
			vecf c1 = v.controls[0].s.gP();
			vecf c2 = v.controls[1].s.gP();
			vecf p1c1 = c1 - pos1;
			vecf c1c2 = c2 - c1;
			vecf c2p2 = pos2 - c2;
			vecf inc1 = p1c1 * .01f;
			vecf inc2 = c1c2 * .01f;
			vecf inc3 = c2p2 * .01f;
			float ct = 0;
			vecf lastPt = pos1;
			for (float j = 1; j <= 100; ++j) {
				if (j == 100) {
					Vert sv {v2};
					sv.setPosition(pos2);
					plat->saveVerts.push_back(sv);
					plat->pts.push_back(pos2);
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
				
				float h = hyp(pos2, pt6);
				if (h < minseg * 2) {
					Vert sv {v2};
					sv.setPosition(pos2);
					plat->saveVerts.push_back(sv);
					plat->pts.push_back(pos2);
					break;
				}
				h = hyp(lastPt, pt6);
				if (h >= avgSeg) {
					Vert sv {v2};
					sv.setPosition(pt6);
					sv.controls.clear();
					plat->saveVerts.push_back(sv);
					plat->pts.push_back(pt6);
				}
				else {
					ct += h;
					if (ct >= avgSeg) {
						Vert sv {v2};
						sv.setPosition(pt6);
						sv.controls.clear();
						plat->saveVerts.push_back(sv);
						plat->pts.push_back(pt6);
						ct = 0;
					}
				}
				lastPt = pt6;
			}
		}
	}
	
	if (gHighlighted) {
		gHighlighted->isHighlighted = false;
		gHighlighted = nullptr;
	}
	plat->isComplete = true;
	plat->verts.shrink_to_fit();
	updateHoleAndTee(plat->recomputeSpline());
	if (makeNew) {
		// MAKE EGROUNDSEG for first vert (starts at last vert)
		newPlatform();
	}
	
	return true;
}

void State::clearMap ()
{
//	std::fstream fs{resourcePath() / "levels" / curPlatFile, std::ios_base::out | std::ios_base::trunc};
//	fs.close();
	curPlatforms.clear();
	gClicked = nullptr;
	gHighlighted = nullptr;
	newPlatform();
}

bool State::maybeEraseSelectedVert ()
{
	if (gHighlighted) {
		auto plat = gHighlighted->parentPlat;
		if (isCmdPressed())
			plat->verts.clear();
		else plat->verts.erase(plat->verts.begin() + indexOfRef(plat->verts, *gHighlighted));
		plat->recomputeSpline();
		if (gHighlighted == gClicked)
			gClicked = nullptr;
		gHighlighted = nullptr;
		return true;
	}
	return false;
}

void State::activateSelectButton ()
{
	curTool = "select";
	toolWin.highlight.sP(rectCenter(toolWin.selectButton));
}

void State::updateHoleAndTee (pair<Vert*, Vert*> ht)
{
	if (ht.first) {
		hole.sP(ht.first->seg.mid);
		hole.setRotation(ht.first->seg.oppAngle);
		flag.sP(hole.gP());
		//	flagTxt.sP(hole.gP() + vecf(0, -51));
	}
	if (ht.second) {
		ball.sP(ht.second->seg.mid + pVec(ball.gLB().height / 2 + 1, ht.second->seg.normal));
	}
}

void State::newPlatform ()
{
	curPlatforms.emplace_back();
	setCurPlat(&curPlatforms.back());
}

void State::deactivateCurTbox ()
{
	if (activeTbox) {
		activeTbox->setActive(false);
		activeTbox = nullptr;
	}
}

void State::setCurPlat (EditorPlatform* ep)
{
	curPlat = ep;
	fillInfoTbox.boxTxt.setString(ep->fillTboxStr);
}

void State::saveHole ()
{
	string tboxtxt = string(filenameTbox.boxTxt.getString());
	if (tboxtxt.empty())
		tboxtxt = "newLevel" + tS(rand());
	string fname = (resourcePath() / "levels" / (tboxtxt + ".txt")).string();
	ofstream fs {fname, std::ios_base::trunc};
	fs << "{ " << int(scrw) << ' ' << int(scrh) << endl;	// log the View size
	for (auto& ep : curPlatforms) {
		if (!finishGround(&ep, false))
			continue;
		vecf start, end;
		auto sz = ep.saveVerts.size();
		for (int i = 1; i < sz; ++i) {
			start = ep.saveVerts[i - 1].s.getPosition();
			end = ep.saveVerts[i].s.getPosition();
			if (ep.saveVerts[i].hasTee)
				fs << "T ";
			else if (ep.saveVerts[i].hasHole)
				fs << "H ";
			fs << start.x << ' ' << start.y << ' '
				<< end.x << ' ' << end.y << ' '
				<< ep.saveVerts[i].surfaceType << endl;
		}
		string ftype = "colordev";
		for (auto ch : ep.fillTboxStr)
			// FOR NOW no fill filenames that have no alphabet
			if (isalpha(ch))
				ftype = "imagepixs";
		fs << ": " << ftype << ' ' << ep.fillTboxStr;

		/*
		string ftype = ep.fillInfo.type == PlatFillInfo::FillType::colorDev ?
		"colordev" : "imagepixs";
		fs << ": " << ftype << ' ';
		if (ftype == "colordev") {
			auto cdi = std::get<PlatFillInfo::ColorDevInfo>(ep.fillInfo.arg);
			fs << cdi.c.r << ' ' << cdi.c.g << ' ' << cdi.c.b << ' '
			<< cdi.c.a << ' ' << cdi.dev << ' ' << cdi.blurRepetitions;
		}
		else {
			fs << std::get<string>(ep.fillInfo.arg);
		}
		*/
		fs << endl;
	}
	fs.close();
	gSound("save").play();
}
