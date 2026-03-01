//
//  zgolf.cpp
//  ZGolf
//
//  Created by John Ziegler on 1/9/25.
//  Copyright © 2025 John Ziegler. All rights reserved.
//

#include "zgolf.hpp"
#include "sfmlApp.hpp"


State* State::instance_ = nullptr;


void State::onCreate ()
{
	instance_ = this;
	
	loadCourses();

	/* Menu setup */
	bkgdSpr.setTexture(gTexture("bkgd"));
	auto sz = bkgdSpr.getTexture()->getSize();
	bkgdSpr.setScale(scrw / sz.x, scrh / sz.y);
	
	menuTitle = Text("ZGolf", gFont("menuTitle"), 230);
	menuTitle.setOutlineThickness(10);
	menuTitle.setOutlineColor(DKORANGE);
	menuTitle.setFillColor(ORANGE75);
	centerOrigin(menuTitle);
	menuTitle.setPosition(scrcx, 200);
	
	signatureTxt = Text(
		"John Ziegler, 2021-2026    johnnywz00@yahoo.com"
						, gFont("toolButton"), 20);
	Color c = ORANGE;
	centerOrigin(signatureTxt);
	signatureTxt.setPosition(scrcx, menuTitle.gP().y + 163);
	signatureTxt.setFillColor(Color(c.r, c.g, c.b, 160));
	
	menuInstr = Text(
					 "Hold Space to shoot\nHold Shift and Space to putt\n"
					 "Arrow keys to pan the view\n"
					 "Aim with mouse or < > keys\n"
					 "\t(Shift speeds up)\n"
					 "N for next hole\n"
					 "M to enter level creator mode"
					 , gFont("toolButton"), 20);
	menuInstr.setOrigin(menuInstr.gGB().width / 2, 0);
	menuInstr.setPosition(scrcx, signatureTxt.gP().y + 60);
	menuInstr.setFillColor(Color(c.r, c.g, c.b, 160));
	
	float buttonX = scrcx - 500;
	forNum(3) {
		string fname = courses[i + 1].holes[0].platformsFile;
		Resources::addTexToMap(make_pair((path("levelSprites") / (fname + ".png")).string(), fname));
		courseButtons.emplace_back(courses[i + 1], vecf(buttonX, scrh - 100)); // i+1 bc dflt course is 0
		buttonX += 500;
	}
	// end menu
	
	toolWin.init();
	loadToolbarButtons();
	
	instrTxt = Text(
					"- The topmost textbox is a filename (without extensions like .txt);"
					"\n\tthis is the file that will be written to when saving a level,"
					"\n\tor the file that will be loaded when returning to play mode\n"
					"- The lower textbox represents how the current platform will be\n"
					"\tcolor-filled. A string will be treated as a filename: the pixels\n"
					"\tof that image will be copied into the boundaries of the platform.\n"
					"\tOtherwise it must be six numbers separated by spaces which stand\n"
					"\tfor R, G, B, and alpha of the primary color, followed by a\n"
					"\tmaximum deviation value from the primary color, followed by the\n"
					"\tnumber of blending iterations to run on the result.\n \n"
					"M  =  switch to play mode, loading the filename in the textbox\n"
					"X  =  clear screen\n"
					"U  =  finalize newest platform and start another (connects the\n"
					"\tlast-placed vertex of the platform with the first one)\n"
					"J  =  save editor as new hole (if name listed in filename textbox\n"
					"\tis new), or permanently apply changes to existing hole (if name\n"
					"\tin textbox is the same as when editor was loaded)\n"
					"Delete  =  erase selected vertex; if Command/Windows key\n"
					"\tpressed, deletes all vertices for that platform\n"
					"Space  =  switch to select tool; deselect selected vertex\n"
					"Enter  =  deselect selected vertex\n"
					
					" \nClick on existing vertex:\n"
					"\t- With select tool, begin to drag vertex for repositioning\n"
					"\t\tWith Command/Windows key pressed, drags all vertices of platform\n"
					"\t- if Shift pressed, highlight or unhighlight the vertex (vert must be\n"
					"\t\thighlighted to create control point for it)\n"
					"\t- With hole button, places hole on the corresponding ground segment\n"
					"\t- With tee button, places tee \" \" \n"
					"\t- With a surface type button, sets the surface type for \" \" \n"
					"\t- if Command/Windows key pressed, sets all segs of that platform to\n"
					"\t\tthat surface type\n"
					
					" \nClick on existing control point:\n"
					"\t- Drag to reposition\n"
					"\t- With Shift pressed, deletes the control point and redraws the spline\n"
					
					" \nClicking in the editor space:\n"
					"\t- Creates a new vertex connected to the last point of the current platform\n"
					"\t- If a vertex is highlighted, creates a new vertex inserted directly \n"
					"\t\tafter the highlighted one\n"
					"\t- If a vertex is highlighted and Shift is pressed, creates a control \n"
					"\t\tpoint for that vertex (two max)\n"
					" \nThe editor doesn't yet have course-awareness: after saving a hole\n"
					"\tit can be added to a course manually by opening the file courses.txt"
					"\n \n                      (Click or press ? to hide)",
					gFont("instr"), 18
					);
	centerOrigin(instrTxt);
	instrTxt.setPosition(scrcx, scrcy);
	instrTxt.setFillColor(CHARCOAL);
	auto ht = instrTxt.gGB().height;
	if (ht > scrh - 10) {
		auto factor = scrh / ht;
		instrTxt.setScale(factor, factor);
	}
	instrRect.setSize({instrTxt.gLB().width + 20, instrTxt.gLB().height + 20});
	centerOrigin(instrRect);
	instrRect.setPosition(scrcx, scrcy);
	instrRect.setFillColor(withAlpha(ORANGE75, 140));
	instrRect.setOutlineThickness(2);
	instrRect.setOutlineColor(ORANGE75);
	
	instrLabel = Text("Press ? key for instructions", gFont("instr"), 18);
	instrLabel.setFillColor(DKORANGE75);
	instrLabel.setPosition(scrw - 510, 23);
	
	statsTxt = Text("", gFont("stats"), 20);
	statsTxt.setPosition(10, 15);
	statsTxt.setOutlineThickness(3);
	statsTxt.setOutlineColor(CHARCOAL);
	statsTxt.setFillColor(Color(200, 200, 210));
	
	mouseTxt = Text("", gFont("debug"), 13);
	mouseTxt.setPosition(8, 65);
	mouseTxt.setFillColor(Color::Black);

	mouseVec.x = Mouse::gP(*rwin).x;
	mouseVec.y = Mouse::gP(*rwin).y;
	
	ball.setTexture(gTexture("ball"));
	centerOrigin(ball);
	ballRadius = ball.gLB().height / 2;
	
	flag.setTexture(gTexture("flag"));
	flag.setOrigin(flag.gLB().left + flag.gLB().width / 2,
				   flag.gLB().top + flag.gLB().height);
	
	flagTxt = Text("", gFont("flagFont"), 13);
	flagTxt.setFillColor(Color::Black);
	
	forNum(10) {
		clouds[i].setTexture(gTexture("cloud"));
		clouds[i].sP(randRange(0, scrw + 400), randRange(0, scrh - 50));
		float factor = float(randRange(50, 120)) / 100;
		clouds[i].setScale(factor, factor);
		cloudVels[i] = float(randRange(5, 30)) / 100;
		int gray = randRange(230, 255);
		clouds[i].setColor(Color(gray, gray, gray));
	}

	curPlatforms.reserve(50);
	filenameTbox = Textbox(gFont("debug"), {scrw - 228, 25});
	fillInfoTbox = Textbox(gFont("debug"), {scrw - 228, 70});
	
	deh.setTexture(gTexture("deh"));
	loadAnimFrames(); //@kludgeAnim
	setDehFrame(0); //@kludgeAnim
	/* Origin will be down near ball position */
	deh.setOrigin(deh.gLB().left + deh.gLB().width - 15, deh.gLB().top + deh.gLB().height - 12);
	deh.setColor(Color(150, 150, 150));
	
	// Trajectory members initialized in loadHole since View size can change
	
	floatEps = .001;
	loadCourse(courses[0]);
	resetGame();
	restoreView(); // Loading the default course/hole will have set view to 1728 1117

// DEBUG ///////////
	drt.create(scrw, scrh);
	VertexArray tempVa {Lines};
	
//	for (auto& p : platforms)
//		for (auto& s : p.segs) {
//						if (!s.next || !s.prev)
//							s.spr.setColor(Color::Black);
//			tempVa.append(VTXC(s.collisionCheckLine.pt1, PURPLE));
//			tempVa.append(VTXC(s.collisionCheckLine.pt2, PURPLE));
//		}
//	drt.draw(tempVa);
//	drt.display();
//	rts.setTexture(drt.getTexture());
} //end onCreate

void State::onMouseDown (int x, int y)
{
	if (mode == menu)
		menuClick(x, y);
	
	else if (mode == design)
		designClick(x, y);
	
	// DEBUG: drop the ball at click loc  ////
	else {
		if (iKP(Tab)) {
			ofstream fs {Resources::executingDir() / "ballstart.txt", ios_base::trunc};
			fs << tS(x) + " " + tS(y);
			fs.close();
			ballActive = true;
			rolling = false;
			gSeg = nullptr;
			ball.sP(x, y);
		}
	}
	//=========
}

void State::onMouseUp (int x, int y)
{
	gClicked = nullptr;
	toolWin.clickDragging = false;
}

void State::onKeyPress (Keyboard::Key k)
{
	switch(k) {
			
		case Keyboard::Escape:
			if (mode == menu)
				app->close();
			else {
				mode = menu;
				restoreView();
			}
			break;
			
		case Keyboard::M:
			mode == design ? switchToPlay() : switchToDesign();
			break;
			
		case Keyboard::Y:
			resetGame();
			break;
			
		default:
			if (mode == design)
				designKeyPress(k);
			else switch(k) {
					
				case Keyboard::T:
					ballActive = !ballActive;
					break;
					
				case Keyboard::N:
					loadNextHole();
					break;
					
			// DEBUG //
				case Keyboard::Slash:
					drt.clear(Color::Transparent);
					break;
					
				case Keyboard::Period:
					testRetro();
					//					makeBlotchTx();
					//					checkForShortSegs(15);
					break;
					
				default:
					break;
			}
	}
}

void State::onKeyRelease (Keyboard::Key) { } // no use yet

void State::update (const Time& time)
{
	timedMgr->fireReadyEvents(time);

	if (mode == menu) {
		// animations, music
		return;
	}
	
	maybePanView();

	if (mode == design)
		designUpdate();
	else playUpdate(time);

	
// DEBUG /////////
	
	++frameCounter;
	if (frameCounter == 2000000)
		frameCounter = 1;
	
	mouseTxt.setString(
					   (curCourse ? curCourse->courseName + " " + tS(curCourse->numHoles) : "NULL") + "\n" + curPlatFile
					   + "\n" + vecfStr(ball.gP()) + "\n" + vecfStr(deh.gP())
					   );
}

void State::draw ()
{
	auto w = rwin;
	if (mode == menu)
		menuDraw();
	
	else if (mode == design)
		designDraw();
	
	else if (mode == play) {

		forNum (10)
			w->draw(clouds[i]);
		w->draw(trajecSpr);
		w->draw(curHoleSprite);
		
		
//		for (auto& p : platforms) { (pre sprite assembly)
//			w->draw(p.s);
//			if (iKP(Num2))
//				w->draw(p.va);  // DBG
//			for (auto& seg : p.segs) {
//				if (!iKP(Num1))  //DBG
//					w->draw(seg.spr);
//                    w->draw(seg.txt);  //DBG
//                    w->draw(seg.n);  //DBG
//			}
//		}
		
		w->draw(flag);
		w->draw(flagTxt);
		w->draw(hole);
		w->draw(deh);
		if (ball.mag() < speedClamp && gSeg)
			w->draw(guideline);
		if (pullingBack) {
			w->draw(powerBar);
			w->draw(powerBarOutline);
		}
		w->draw(ball);
		w->draw(statsTxt);
	
	// ///////DEBUG
//	w->draw(rr);
//	w->draw(gsva);
//	w->draw(rrr);
	// //////////////////
	}
//	w->draw(mouseTxt);
}

void State::loadAnimFrames()
{
	int orderNums[] = {2, 1, 0, -1, -2, -3, -4, -5, -6};
	forNum(9) {
		IntRect rect {62 * (i % 4), 53 * (i / 4), 62, 53};
		swingFrames.emplace_back(rect, orderNums[i]);
	}
}

void State::resetGame ()
{
	if (mode == play)
		ballActive = true;
	else
		ballActive = false;
	ball.setVelocity(0, 0);
	
	// DEBUG BALL START LOCATION
	{
		//		ball.sP(ScrCX, ScrCY);
		
		//		ifstream fs {Resources::executingDir() / "ballstart.txt"};
		//		string line;
		//		getline(fs, line);
		//		stringstream ss(line);
		//		string tok;
		//		ss >> tok;
		//		vecf pos;
		//		pos.x = stof(tok);
		//		ss >> tok;
		//		pos.y = stof(tok);
		//		fs.close();
		//		ball.sP(pos);
	}
	
	gClicked = nullptr;
	gHighlighted = nullptr;
	gSeg = nullptr;
	rolling = false;
	putting = false;
	pullingBack = false;
	inCrotch = false;
	onCusp = false;
	powerRising = true;
	
	// DEBUG ///////
	//	ballActive = false;
}

/* resources/levels/ needs to contain a file courses.txt
 * which lists the courses to load
 */
void State::loadCourses()
{
	// Temp container for levels created in editor, until editor gets course handling
	Course dfltCourse;
	dfltCourse.courseName = "default";
	dfltCourse.numHoles = 1;
	CourseHole dfltHole;
	dfltHole.platformsFile = "platforms";
	dfltHole.holeNumber = 1;
	dfltCourse.holes.push_back(dfltHole);
	
	// count and reserve or keep total count current in data file
	ifstream courseFile {resourcePath() / "levels" / "courses.txt"};
	if (!courseFile.is_open()) {
		cerr << "Couldn't load 'levels/courses.txt'. \n";
		courses.push_back(dfltCourse);
		return;
	}
	
	string line;
	Course course;
	while(getline(courseFile, line)) {
		if (line.empty() || line[0] == '#')
			continue;
		else if (isdigit(line[0])) {
			courses.reserve(stoi(line) + 1); // +1 for default course
			courses.push_back(dfltCourse);
		}
		else if (line[0] == ':') {
			course.numHoles = (int)course.holes.size();
			for (auto& hole : course.holes)
				course.par += hole.par;
			courses.push_back(course);
			course = Course();
		}
		else if (line[0] == '@') {
			course.courseName = line.substr(1);
		}
		else {
			CourseHole chl;
			chl.platformsFile = line;
			chl.holeNumber = (int)course.holes.size() + 1;
			//par
			course.holes.push_back(chl);
		}
	}
	courseFile.close();
}

void State::loadCourse(Course& course)
{
	curCourse = &course;
	course.reset();
	if (course.numHoles)
		loadNextHole();
}

void State::loadHole (CourseHole& cHole)
{
	cHole.strokeCt = 0;
	flagTxt.setString(tS(cHole.holeNumber));
	centerOrigin(flagTxt);
	loadPlatforms(cHole.platformsFile, &cHole);
	auto sz = cHole.viewSize; // viewSize set within loadPlatforms()
	rwin->setView(View(FloatRect(0, 0, sz.x, sz.y)));
	trajecRt.create(sz.x, sz.y);
	trajecImg.create(sz.x, sz.y, Color::Transparent);
	trajecTx.loadFromImage(trajecImg);
	trajecSpr.setTexture(trajecTx);
	curPlatFile = cHole.platformsFile;
	moveDehToBall();
	deh.setScale(hole.gP().x > ball.gP().x ? 1 : -1, 1);
	angle = 270;
	timedMgr->gSet("canShoot");
	teeingOff = true;
	ballActive = true;
	
	// ///// FOR DBG, set sky gray bc ball activity will set
	// sky different colors; else random choice till courses
	// dictate their own sky color
	vector<Color> skyColors {
#ifdef DBG
		RAINYGRAY
#else
		CORNFLOWER, SKYBLUE
//		, DRABCYAN, DKAZURE
//		, BUTTERSKY, PEACH
#endif
	};
	app->setRedrawColor(randElemVal(skyColors));
}

void State::loadNextHole()
{
//	if (curCourse->nextHole > curCourse->numHoles) {
//		//final score, ending details
//
//	}
//	else
	{
		if (curCourse->nextHole > curCourse->numHoles) // // temp: keep cycling holes
			curCourse->nextHole = 1;
		
		CourseHole& cHole = curCourse->holes[curCourse->nextHole - 1];
		curCourse->curHole = &cHole;
		++curCourse->nextHole;
		loadHole(cHole);
	}
}

void State::loadPlatforms (string fname, CourseHole* chl)
{
	platforms.clear();
	ifstream plats {resourcePath() / "levels" / (fname + ".txt")};
	if (!plats.is_open()) {
		cerr << "Couldn't load " + fname + ". \n";
		return;
	}
	string line;
	
	int platCt = 0;
	int curSegCt = 0;
	intvec segCts;
	bool hasCachedSprite = false;
	while (getline(plats, line)) {
		if (isdigit(line[0]) || line[0] == 'T' || line[0] == 'H')
			++curSegCt;
		else if (line.find(':') != line.npos) {
			++platCt;
			segCts.push_back(curSegCt);
			curSegCt = 0;
		}
		else if (line == "SPRITE_CACHED")
			hasCachedSprite = true;
	}
	if (platCt == 0)
		return;
	platforms.reserve(platCt);
	platCt = 0;
	resetGetline(plats);
	
	Platform p;
	p.segs.reserve(segCts[platCt]);
	while (getline(plats, line)) {
		if (line.empty() || line[0] == '#')
			continue;
		stringstream ss {line};
		string tok;
		if (line[0] == '{') {
			if (chl) {
				ss >> tok >> chl->viewSize.x >> chl->viewSize.y;
			}
			continue;
		}
		/* Colon in config file signals end of a platform.
		 * Make sure to connect the last vertex to the first vertex,
		 * then create a sprite from the ground segment data, and empty
		 * the platform variable for the next one.
		 */
		if (line[0] == ':') {
			PlatFillInfo pfi;
			ss >> tok; // Pass over colon
			if (ss >> tok) {
				if (tok == "colordev") {
					pfi.type = PlatFillInfo::FillType::colorDev;
					uint red, green, blue, alpha;
					int dev, reps;
					ss >> red >> green >> blue >> alpha >> dev >> reps;
					PlatFillInfo::ColorDevInfo cdi {
						Color(red, green, blue, alpha), dev, reps
					};
					pfi.arg = cdi;
				}
				else if (tok == "imagepixs") {
					pfi.type = PlatFillInfo::FillType::imagePixs;
					ss >> tok;
					pfi.arg = tok;
				}
			}
			p.fillInfo = pfi;
			
			if (p.va.getVertexCount()) {
				p.va.append(Vertex(p.va[0].position, p.fillColor)); //127, 167, 53)));
			}
			platforms.push_back(p);
			auto& pl = platforms.back();
			forNum (pl.segs.size()) {
				/* Use modulo operator to make segments link cyclically */
				pl.segs[i].next = &(pl.segs[(i + 1) % pl.segs.size()]);
				pl.segs[i].prev = &(pl.segs[(i + pl.segs.size() - 1) % pl.segs.size()]);
				pl.segs[i].setConcavityToNeighbors();
			}
			pl.va.configure();
			
			++platCt;
			if (platCt == segCts.size())
				break;
			p = Platform();
			p.segs.reserve(segCts[platCt] + 2);
			continue;
		}
		
			/* It's a line of numbers representing the start and end coordinates
			 * of a single ground segment to be added to current platform
			 */
		vecF start, end;
		bool hasHole = false;
		bool hasTee = false;
		string surfaceType;
		auto peek = ss.peek();
		if (peek == 'T' || peek == 'H') {
			ss >> tok;
			if (tok == 'T')
				hasTee = true;
			else if (tok == 'H')
				hasHole = true;
		}
		
		ss >> start.x >> start.y >> end.x >> end.y;
		GroundSegment g {start, end, ballRadius};
		if (ss >> tok) {
			surfaceType = tok;
		}
		else surfaceType = g.facesUp ? "grass" : "dirt";
		g.surfaceType = surfaceType;
		g.initSprite(gTexture(surfaceType));
		g.txt.setFont(gFont("debug"));
		g.muK = physicsMap[surfaceType].muK;
		g.muS = physicsMap[surfaceType].muS;
		g.bounceLoss = physicsMap[surfaceType].bounceLoss;
		if (hasHole) {
			hole.sP(g.mid);
			hole.setRotation(g.oppAngle);
			flag.sP(g.mid);
			flagTxt.sP(g.mid + vecf(0, -51));
		}
		if (hasTee) {
			ball.sP(g.mid + pVec(ballRadius + 1, g.normal));
		}
		p.segs.push_back(g);
		p.va.append(Vertex(start, p.fillColor));
	} // end while
	plats.close();
	
	strPair	txInfo = {(path("levelSprites") / (fname + ".png")).string(), fname};
	if (!hasCachedSprite) {
		assembleSprite(fname);
		/* Always reload the texture if it's been modified in the editor */
		Resources::addTexToMap(txInfo);
	}
	if (!Resources::texExists(fname))
		Resources::addTexToMap(txInfo);
	curHoleSprite.setTexture(gTexture(fname));
}

void State::menuDraw ()
{
	rwin->draw(bkgdSpr);
	for (auto& cb : courseButtons)
		rwin->draw(cb);
	rwin->draw(menuTitle);
	rwin->draw(signatureTxt);
	rwin->draw(menuInstr);
}

void State::menuClick (int x, int y)
{
	for (auto& btn : courseButtons) {
		if (btn.bkgdSky.gGB().contains(x, y)) {
			mode = play;
			loadCourse(*(btn.course));
		}
	}
}

void State::switchToPlay ()
{	
	mode = play;
	try {
		string arg = filenameTbox.boxTxt.getString();
		if (arg.empty()) {
			arg = "platforms";
			ofstream os {resourcePath() / "levels" / (arg + ".txt"), std::ios_base::app};
			os.close();
		}
		if (holeWhenEnteringMode
			&& arg == holeWhenEnteringMode->platformsFile) {
			loadHole(*holeWhenEnteringMode);
		}
		// Editor doesn't have course recognition yet; put a new hole
		// in the default course
		else {
			CourseHole chl;
			chl.platformsFile = arg;
			chl.par = 4;
			chl.holeNumber = int(courses[0].holes.size() + 1);
			courses[0].holes.push_back(chl);
			loadHole(courses[0].holes.back());
			curCourse = &courses[0];
			curCourse->curHole = &curCourse->holes.back();
		}
	}
	catch (std::exception& e) {
		ofstream errFile { "errlog.txt", std::ios_base::app};
		cerr << LocalTime().ascTime() << " Caught exception: " << e.what() << endl;
	}
}

void State::assembleSprite (string fname)
{
	RenderTexture rt, platRt, segRt;
	TransformableVxArray va {LineStrip};
	Texture tex;
	Sprite spr, platSpr;
	
	rt.create(scrw, scrh); // CHANGE if adding panning
	platRt.create(scrw, scrh);
	rt.clear(Color::Transparent);
	
	for (auto& p : platforms) {
		platRt.clear(Color::Transparent);
		/* First draw an outline of the platform */
		platRt.draw(p.va);
		platRt.display();
		ZImage zimg {platRt.getTexture().copyToImage()};
		/* Find a point that's inside the platform bounds */
		vecf startPtFl {p.segs[1].mid};
		vecU startPt;
		do {
			startPtFl += toRect(1, p.segs[1].oppNormal);
			startPt.x = (uint)startPtFl.x;
			startPt.y = (uint)startPtFl.y;
		}
		while (!zimg.isBlank(zimg.getPixel(startPt)));
		/* Use "fill bucket" algorithm to color in the platform */
		if (p.fillInfo.type == PlatFillInfo::FillType::imagePixs) {
			zimg.fillInFromImage(startPt, (resourcePath() / "images" / (std::get<string>(p.fillInfo.arg) + ".png")).string());
		}
		// more types like random sprite sprinkling
		else { // FillType::colorDev
			auto cdi = std::get<PlatFillInfo::ColorDevInfo>(p.fillInfo.arg);
			zimg.fillInWithColor(startPt, cdi.c, cdi.dev);
			zimg.blur(cdi.blurRepetitions);
		}
		
		tex.loadFromImage(zimg);
		platRt.draw(Sprite(tex));
		
		/* Then draw the surface segments over and around the platform body */
		for (auto& seg : p.segs) {
			/* Most surface types we'll just bisect between neighboring
			 * segments, but some will use an "end cap"
			 */
			auto ySize = gTexture(seg.surfaceType).getSize().y;
			auto yAboveOrigin = ySize - ballRadius;
			float nextAng = bisectSmallest(seg.angle, seg.next->angle);
			float prevAng = bisectSmallest(seg.angle, seg.prev->angle);
			if (Resources::texExists(seg.surfaceType + "End")) {
				if (seg.next->surfaceType != seg.surfaceType)
					nextAng = seg.angle;
				if (seg.prev->surfaceType != seg.surfaceType)
					prevAng = seg.angle;
			}
			
		/* Lots of work and calculations to get the bisections right;
		 * ultimately we have to create a VertexArray of lines that
		 * outline our segment with each end correctly bisected with
		 * respect to its neighbor: then using ZImage we first fill
		 * that in solid and finally map the appropriate surface
		 * texture pixels to only those pixels which have been filled
		 * in solid.
		 */
			float trAng = prevAng + 90;
			float brAng = prevAng + 270;
			float tlAng = nextAng + 90;
			float blAng = nextAng + 270;
			
			vecf tr = seg.start + pVec(ballRadius / max(.1f, absCos(trAng, seg.normal)), trAng);
			vecf br = seg.start + pVec(yAboveOrigin / max(.1f, absCos(brAng, seg.oppNormal)), brAng);
			vecf tl = seg.end + pVec(ballRadius / max(.1f, absCos(tlAng, seg.normal)), tlAng);
			vecf bl = seg.end + pVec(yAboveOrigin / max(.1f, absCos(blAng, seg.oppNormal)), blAng);
			
			auto trdif = toPolar(tr - seg.start);
			auto brdif = toPolar(br - seg.start);
			auto tldif = toPolar(tl - seg.start);
			auto bldif = toPolar(bl - seg.start);
			
			auto tr2 = seg.start + pVec(trdif.x, trdif.y - seg.angle);
			auto br2 = seg.start + pVec(brdif.x, brdif.y - seg.angle);
			auto tl2 = seg.start + pVec(tldif.x, tldif.y - seg.angle);
			auto bl2 = seg.start + pVec(bldif.x, bldif.y - seg.angle);
			
			va.clear();
			va.appendPtC(tr2, CHARCOAL);
			va.appendPtC(br2, CHARCOAL);
			va.appendPtC(bl2, CHARCOAL);
			va.appendPtC(tl2, CHARCOAL);
			va.appendPtC(tr2, CHARCOAL);
			va.configure();
			auto bounds = va.getBounds();
			auto dif = vecf(-bounds.left, -bounds.top);
			vecf ogn = seg.start - vecf(bounds.left, bounds.top);
			va.move(dif);
			
			segRt.create(bounds.getSize().x, bounds.getSize().y);
			segRt.clear(Color::Transparent);
			segRt.draw(va);
			segRt.display();
			zimg = segRt.getTexture().copyToImage();
			auto fillPtF = ogn + vecf(4, 0);
			vecU fillPt = {(uint)fillPtF.x, (uint)fillPtF.y};
			zimg.fillInWithColor(fillPt, CHARCOAL);
			zimg.fillInFromImage(fillPt, (resourcePath() / "images" / (seg.surfaceType + ".png")).string());
			tex.loadFromImage(zimg);
			spr.setTexture(tex);
			spr.setTextureRect(IntRect(0, 0, tex.getSize().x, tex.getSize().y));
			spr.setOrigin(ogn);
			spr.setPosition(seg.start);
			spr.setRotation(seg.angle);
			platRt.draw(spr);
		}
		/* Second pass to draw "end caps" where applicable */
		for (auto& seg : p.segs) {
			string endkey = seg.surfaceType + "End";
			if (Resources::texExists(endkey)) {
				if (seg.next->surfaceType != seg.surfaceType) {
					Sprite endspr(gTexture(endkey));
					endspr.setOrigin(0, ballRadius);
					endspr.setRotation(seg.angle);
					endspr.setPosition(seg.end);
					endspr.setScale(1, -1);
					platRt.draw(endspr);
				}
				if (seg.prev->surfaceType != seg.surfaceType) {
					Sprite endspr(gTexture(endkey));
					endspr.setOrigin(0, ballRadius);
					endspr.setRotation(seg.angle);
					endspr.setPosition(seg.start);
					endspr.setScale(-1, -1);
					platRt.draw(endspr);
				}
			}
		}
		platRt.display();
		// Use these two lines if dynamic platforms introduced
		//		p.tx = platRt.getTexture();
		//		p.s.setTexture(p.tx);
		platSpr.setTexture(platRt.getTexture());
		platSpr.setTextureRect(IntRect(0, 0, scrw, scrh));
		rt.draw(platSpr);
	}
	rt.display();
	rt.getTexture().copyToImage().saveToFile((resourcePath() / "images" / "levelSprites" / (fname + ".png")).string());
	ofstream ofs { resourcePath() / "levels" / (fname + ".txt"), std::ios_base::app };
	ofs << "\nSPRITE_CACHED\n";
	ofs.close();
}

void State::playUpdate (const Time& time)
{
	//==  DEBUG CONTROLS  ===============
	adjustVal(Z, muK, .01, 0, 5)
	adjustVal(X, muS, .01, 0, 5)
	adjustVal(Num1, bounceLoss, .02, 0, 1)
	adjustVal(Num2, maxAngForRoll, 1, 0, 359)
	adjustVal(Num3, maxAngForRollCvx, 1, 0, 359)
	adjustVal(Num4, convexRollClamp, .02, 0, 5)
	adjustVal(Num5, centrifugalDecmFactor, .02, 0, 1)
	adjustVal(Num6, centrifugalIncmFactor, .02, 0, 1)
		
	bool dbgMove = false;
	if (isShiftPressed() && iKP(D)) { ball.move(2, 0); dbgMove = true; }
	if (isShiftPressed() && iKP(A)) { ball.move(-2, 0); dbgMove = true; }
	if (isShiftPressed() && iKP(W)) { ball.move(0, -2); dbgMove = true; }
	if (isShiftPressed() && iKP(S)) { ball.move(0, 2); dbgMove = true; }
	if (dbgMove) {
		rolling = false;
		gSeg = nullptr;
	}
	//====  end debug controls  =======

	/* Move clouds */
	forNum (10) {
		clouds[i].move(-cloudVels[i], 0);
		if (clouds[i].gGB().left + clouds[i].gGB().width < -1)
			clouds[i].sP(curCourse->curHole->viewSize.x + 2, clouds[i].gP().y);
	}
	
	handleAim();
	
	if (ballActive) {
		if (timedMgr->gOn("canShoot") && gSeg) {
			updateGuide();
			
			if (iKP(Space))
				handleSwing();
			
			else if (!iKP(Space) && pullingBack)
				startDownswing();
		}
		
		ball.addVelocity(vGravity);
		
		if (rolling) {
			if (!gSeg) {
				ballActive = false;
				zeroOutVelocity();
				dbgMsg = "  ROLL CALLED WHILE GSEG NULL";
			}
			/* roll() itself will transition to fly() if the newly added gravity
			 * has pulled the ball away from the surface
			 */
			roll(1.f);
		}
		else fly(1.f);
	}
	
	fadeTrajectory();
	
	statsTxt.setString(
					   "COURSE: " + curCourse->courseName +
					   "      HOLE: " + tS(curCourse->curHole->holeNumber) +
					   "     PAR: " + tS(curCourse->curHole->par) +
					   "\nSTROKES: " + tS(curCourse->curHole->strokeCt) +
					   "     TOTAL: " + tS(curCourse->strokeCt)
					   );

	// DEBUG ////////

	//drt.clear(Color::Transparent); ?

	mouseTxt.setString(
					   tS(mouseVec.x) + ", " + tS(mouseVec.y)
					   //					   fS(ball.gP().x) + ", " + fS(ball.gP().y)
					   //					   tS(powerBarOutline.getVertexCount())
					   + (mode == design ? " DESIGN" : " PLAY") + " " + fS(power)
					   + "\nMove ball: DAWS\nMode: M\nClear Map: X\nFinish Ground: U\nReset: Y\nAim: Left/Right\nShoot: SPACE\nPutt: SHIFT+SPACE\n"
					   + "muK: Z + (" + fS(muK, 2) + ")\nmuS: X + (" + fS(muS, 2) + ")"
					   +"\n bounceLoss(1): "+fS(bounceLoss,2)
					   +"\n maxCcvRoll(2): "+fS(maxAngForRoll)
					   +"\n maxCvxRoll(3): "+fS(maxAngForRollCvx)
					   +"\n cvxRollClamp(4): "+fS(convexRollClamp,2)
					   +"\n centrifDcm(5): "+fS(centrifugalDecmFactor,2)
					   +"\n centrifIcm(6): "+fS(centrifugalIncmFactor,2)
					   
					   +"\n\n"+dbgMsg
					   );
} //end update

void State::maybePanView ()
{
	// Panning the screen: not useful till editor supports
	View vw = rwin->getView();
	auto oldPos = vw.getCenter();
	auto leftLimit = curCourse->curHole->viewSize.x / 2;
	auto topLimit = curCourse->curHole->viewSize.y / 2;
	bool changedView = false;
	if (iKP(Left)) {
		vw.move(-5, 0);
		changedView = true;
		if (vw.getCenter().x < leftLimit)
			vw.setCenter(leftLimit, vw.getCenter().y);
	}
	if (iKP(Up)) {
		vw.move(0, -5);
		changedView = true;
		if (vw.getCenter().y < topLimit)
			vw.setCenter(vw.getCenter().x, topLimit);
	}
	if (iKP(Right)) {
		vw.move(5, 0);
		changedView = true;
	}
	if (iKP(Down)) {
		vw.move(0, 5);
		changedView = true;
	}
	if (changedView) {
		auto dif = vw.getCenter() - oldPos;
		statsTxt.move(dif);
		rwin->setView(vw);
	}
}

void State::handleAim ()
{
	/* Mouse for shot aim */
	// This was an effort to make aiming require some visual
	// judgment rather than simply placing the cursor at the
	// point you want to aim for, but I suspect it may frustrate
	// the casual user, and it got finicky near the borders of
	// the screen
	/*
	 float oldAng = angle;
	 float aimRadius = 400;
	 vecf oldVec = pVec(aimRadius, oldAng);
	 vecf mouseDif = toVecF(mouseVec - oldMouse);
	 vecf newVec = oldVec + mouseDif;
	 angle = toPolar(newVec).y;
	 */
	
	if (gSeg) {
		float oldAng = angle;
		// ////////////////
		// THIS WAS DEBUG MOUSE AIM: but for now using it for release also
		if (mouseVec != oldMouse) {
			vecf dif = vecf(mouseVec.x, mouseVec.y) - ball.gP();
			angle = toPolar(dif).y;
		}
		// ////////////
		
		/* Keyboard control overrides mouse if used */
		bool keyboardUsed = false;
		if (iKP(Comma) && !iKP(Period)) {
			angle -= (isShiftPressed() ? angleRate * 2 : angleRate);
			keyboardUsed = true;
		}
		if (iKP(Period) && !iKP(Comma)) {
			angle += (isShiftPressed() ? angleRate * 2 : angleRate);
			keyboardUsed = true;
		}

		/* Can't iron shoot nearly parallel with surface */
		float leftThresh = gSeg->angle;
		float rightThresh = gSeg->oppAngle;
		if (inCrotch) {
			leftThresh = crotchInfo.nextSeg()->angle;
			rightThresh = crotchInfo.prevSeg()->angle;
		}
		leftThresh = czdg(leftThresh + minAngForIronShot);
		rightThresh = czdg(rightThresh - minAngForIronShot);
		if (angleIsOrFallsBetween(angle, rightThresh, leftThresh)) {
			angle = angleBetween(angle, leftThresh) < angleBetween(angle, rightThresh) ? leftThresh : rightThresh;
		}
		/* Can't shoot nearly vertical */
		float vertLeftThresh = 270 - ironMinDevFromVertical;
		float vertRightThresh = 270 + ironMinDevFromVertical;
		if (angleIsOrFallsBetween(angle, vertLeftThresh, vertRightThresh)) {
			if (keyboardUsed)
				angle = clockwiseOf(angle, oldAng) ? vertRightThresh : vertLeftThresh;
			else	// THIS may not be the right logic if going back to other mouse aim
				angle = angleBetween(angle, vertLeftThresh) < angleBetween(angle, vertRightThresh) ? vertLeftThresh : vertRightThresh;
		}
	}
}

void State::updateGuide ()
{
	guideline.clear();
	vecF ogn = ball.gP();
	for (int i = 0; i < 200; i += 8) {
		uint grayVal = 50;
		if (getBrightness(app->redrawColor) < 60)
			grayVal = 230;
		Color c = Color(grayVal, grayVal, grayVal);
		guideline.appendPtC(ogn + pVec(i, angle), c);
		guideline.appendPtC(ogn + pVec(i + 2, angle), c);
	}
	
	moveDehToBall();
	if (angleIsOrFallsBetween(angle, 90, 270))
		deh.setScale(-1, 1);
	else deh.setScale(1, 1);
}

void State::handleSwing ()
{
	if (!pullingBack) {
		pullingBack = true;
		powerRising = true;
		if (isShiftPressed() || gSeg && physicsMap[gSeg->surfaceType].puttOnly)
			putting = true;
	}
	if (powerRising) {
		if (power < maxPct)
			power = incm(power, powerRate, maxPct);
		else powerRising = false;
	}
	else {
		if (power > 0)
			power = decm(power, powerRate);
		else powerRising = true;
	}
	updatePowerBar(power / maxPct);
		
	int frameNum = 0;	//@kludgeAnim
	if 		(power > 90) frameNum = -6;
	else if (power > 72) frameNum = -5;
	else if (power > 54) frameNum = -4;
	else if (power > 36) frameNum = -3;
	else if (power > 18) frameNum = -2;
	else if (power > 0)  frameNum = -1;
	
	if (putting)
		frameNum = max(frameNum, -2);
	setDehFrame(frameNum);
}

void State::updatePowerBar(float pct)
{
	float lRMultiplier = angleIsOrFallsBetween(angle, 90, 270) ? -1 : 1;
	float startDegAbs = 45;
	float startDeg = 90 + startDegAbs * lRMultiplier * -1; // Dev. from 90°
	float degreeRange = 270 - startDegAbs;
	int arcIncrementCt = 50;
	float arcIncrement = degreeRange / arcIncrementCt;
	uint pbAlpha = 180;
	float rad1 = 100
		, rad2 = rad1 + 20
		, rad3 = rad1 + 2
		, rad4 = rad2 - 2
	;
	auto ogn = ball.gP();
	
	/* Build outline */ //THIS COULD BE MADE INTO A SPRITE IN INIT
	powerBarOutline.clear();
	auto f = [&](float mag, float dir) {
		powerBarOutline.append(VTXC(ogn + pVec(mag, dir), Color(20, 20, 24, pbAlpha)));
	};
	f(rad2, startDeg);
	forNum(arcIncrementCt) {
		f(rad1,
		  czdg(startDeg + (i * arcIncrement) * lRMultiplier)
		  );
	}
	forNum(arcIncrementCt) {
		f(rad2,
		  czdg(startDeg + ((arcIncrementCt - 1 - i) * arcIncrement) * lRMultiplier)
		  );
	}
	/* Build indicator */
	powerBar.clear();
	auto f2 = [&](float mag, float dir, Color c) {
			powerBar.append(VTXC(ogn + pVec(mag, dir), c));
		};
	float arcIcmPct = arcIncrement * pct;
	forNum(arcIncrementCt) {
		f2(rad3,
		   czdg(startDeg + (i * arcIcmPct) * lRMultiplier),
		   Color(255, 159, 63, pbAlpha)
		   );
		f2(rad4,
		   czdg(startDeg + (i * arcIcmPct) * lRMultiplier),
		   Color(255, 0, 0, pbAlpha)
		   );
	}
}

void State::startDownswing ()
{
	if (putting && curFrameNum == 1
		|| curFrameNum == 2) {
		timedMgr->addEvent(2, [&]() {
			setDehFrame(0);
			putting = false;
		});
		//		putting = false;
		/* Causing putt animation to follow through as far as iron swing: are
		 * copies of Fuses or FusePtrs being made and running directly after this */
		return;
	}
	setDehFrame(curFrameNum + 1);
	if (curFrameNum == 0)
		launch();
	timedMgr->addEvent(putting ? .2 : .1, [&]() { startDownswing(); });
}

void State::setDehFrame (int orderNum) //@kludgeAnim
{
	auto& frame = swingFrames[indexWhich(swingFrames,
			[orderNum](auto x) { return x.orderNum == orderNum; }
										 )];
	deh.setTextureRect(frame.subRect);
	curFrameNum = frame.orderNum;
}

void State::launch ()
{
	if (putting && gSeg) {
		ball.setMag(maxPuttPower / 100 * power);
		if (inCrotch) {
			ball.setDirec(
						  angle > 270 || angle < 90 ? crotchInfo.prevSeg()->oppAngle : crotchInfo.nextSeg()->angle
						  );
			gSeg = angle > 270 || angle < 90 ? crotchInfo.prevSeg() : crotchInfo.nextSeg();
		}
		else ball.setDirec(
						  angle > 270 || angle < 90 ? gSeg->oppAngle :  gSeg->angle);
		rolling = true;
	}
	
	else {
		float maxVal = gSeg ? physicsMap[gSeg->surfaceType].maxPower : dfltMaxPower;
		ball.setVelocityP(maxVal / 100 * power, angle);
		endRoll();
	}
	
	++curCourse->strokeCt;
	++curCourse->curHole->strokeCt;
	ballActive = true;
	teeingOff = false;
	pullingBack = false;
	inCrotch = false;
	onCusp = false;
	disableShooting();
	power = 0;
	gSound(putting ? "putt" : "swing").play();
	// putting is set false by Fuse
}

void State::fly(float pct)
{
	disableShooting();
	
	if (ball.mag() < speedClamp) {
		ball.setVelocity(0, 0);
		
		rr.setFillColor(Color::Red); storedFrame = frameCounter;// /////
		return;
	}
//	if (inCrotch) {  }
	inCrotch = false;
	onCusp = false;
	
	if (pct < .98) { rr.setFillColor(Color::Green); storedFrame = frameCounter; }// ////
	else if (!storedFrame || frameCounter - storedFrame > 8) // ////////
		rr.setFillColor(Color::Blue);// /////

	//add wind vector
	
	auto oldPos = ball.gP();
	auto newPos = oldPos + ball.velocity * pct;

	// KEEPING BALL IN SCREEN: DIFFERENT APPROACH ///////////
	{
		auto rightBound = curCourse->curHole->viewSize.x;
		auto bottomBound = curCourse->curHole->viewSize.y;
		if (newPos.x < 2 || newPos.x > rightBound - 2) {
			ball.sP(newPos.x < 0 ? 2 : rightBound - 2, ball.gP().y);
			ball.setDx(ball.dx() * -1);
//			ball.setMag(ball.mag() * 1.3);
		}
		if (newPos.y > bottomBound) {
			ball.sP(ball.gP().x, bottomBound);
			zeroOutVelocity();
		}
	}
	//////////////
	
	LineSegment path {oldPos, newPos};
	CollisionInfo ci;
	
	{ // temp block because kids wanted deh-deh to say "ow" if ball hits him
//		auto rect = deh.gGB();
//		rect.height -= rect.height / 2 + 4;
//		rect.left += 8;
//		rect.width -= 16;
//		if (rect.contains(ball.getPosition().x, ball.getPosition().y)) {
//			gSound("ow").play();
//			timedMgr->addEventIf("ow", 2, [&](){ gSound("crying").play(); });
//		}
	}
	
	/* Collision check with platforms */
	for (auto& p : platforms) {
		
		/* If ball is not in range of this platform, skip it */
		
		// //THIS CHECK WASN'T CATCHING ACCURATELY WHEN THE ADDED MARGIN WAS 2.
		//could potentially still miss if speed is high and the path is crossing just a corner of the platform bounds rect
		
		auto rec = rectWithAddedMarginOf(p.va.getBounds(), 20);
		if (!path.intersectsWith(rec)
			&& !rec.contains(oldPos)
			&& !rec.contains(newPos))
			continue;
		
		/* In bounds of this platform: check each segment of it for collision */
		for (auto& s : p.segs) {

			 /* Disregard segs if the ball is not approaching from a valid
			  * collision direction.
			  */
			if (!angleIsOrFallsBetween(ball.direc(), s.oppAngle, s.angle))
				continue;

			vecf isctPt;
			/* First check for straightforward collisions in the middle of a
			 * surface or near concave segment ends
			 */
			
			if (iKP(U))
				pauseAfterDraw = true;
			
//			vecf stdif = toPolar(s.collisionCheckLine.pt1 - oldPos);
//			vecf edif = toPolar(s.collisionCheckLine.pt2 - oldPos);
//			bool intersects = s.ptIsNormalFromCheckline(oldPos) && !s.ptIsNormalFromCheckline(newPos) &&
//				clockwiseOf(edif.y, path.angle) &&
//				!clockwiseOf(stdif.y, path.angle);
			
			LineSegment s1 {oldPos, s.collisionCheckLine.pt1};
			LineSegment s2 {s.collisionCheckLine.pt1, newPos};
			LineSegment s3 {newPos, s.collisionCheckLine.pt2};
			LineSegment s4 {s.collisionCheckLine.pt2, oldPos};
			bool intersects =
				clockwiseOf(s2.angle, s1.angle)
			&& clockwiseOf(s3.angle, s2.angle)
			&& clockwiseOf(s4.angle, s3.angle)
			&& clockwiseOf(s1.angle, s4.angle);
			
//			if (path.intersectsWith(s.collisionCheckLine, &isctPt)) {
			if (intersects) {
				isctPt = path.intersectionPointWith(s.collisionCheckLine);
				if (isnan(isctPt.x)
					|| isnan(isctPt.y)
					|| isinf(isctPt.x)
					|| isinf(isctPt.y))
					continue;
				/* Store this if it's nearer than any other projected
				 * collision so far
				 */
				if (ci.seg == nullptr
					|| hyp(oldPos, isctPt) < hyp(oldPos, ci.collisionPt))
					ci = CollisionInfo(&s, isctPt);
			}
			
//			else if (path.intersectsWith(rectWithAddedMarginOf(s.bounds, 5))) {
//			else if (
//					(clockwiseOf(s2.angle, s1.angle) || angleBetween(s2.angle, s1.angle) < 10 )
//				 && (clockwiseOf(s3.angle, s2.angle) || angleBetween(s3.angle, s2.angle) < 10 )
//				 && (clockwiseOf(s4.angle, s3.angle) || angleBetween(s4.angle, s3.angle) < 10 )
//				 && (clockwiseOf(s1.angle, s4.angle) || angleBetween(s1.angle, s4.angle) < 10 )
//					 ) {
//				RectangleShape rec;
//				rec.setFillColor(PURPLE75);
//				rec.setSize({s.length + 10, 10});
//				rec.setOrigin(5,5);
//				rec.setPosition(s.start);
//				rec.setRotation(s.angle);
//				Sprite spr;
//				spr.sP(oldPos);
//				int eighth = s.length / 2;
//				auto pathDif = newPos - oldPos;
//				auto step = pathDif / (float)eighth;
//				forNum(eighth) {
//					spr.sP(oldPos + (float)i * step);
//					if (rotatedContains(rec, spr.gP().x, spr.gP().y)) {
//						rec.setFillColor(Color::Black);
//						break;
//					}
//				}
//				drtDraw(rec);
//			}
			
			/* Now check for collisions with convex segment ends where
			 * line segment/arc intersections require quadratic computation.
			 */
#if 0	// (CURRENTLY DISREGARDING)
			else {
				/* No need to enter the block unless this segment has a convex end */
				if (!s.concaveFromPrev) {
					/* Do a lightweight check first */
					if (hyp(s.start, path.pointPerpendicularTo(s.start)) <= 		ballRadius) {
						/* The path does cross near the segment end point: now
						 * we need to know the actual intersection point(s)
						 */
						auto pts = s.lseg.intersectionPointsWith(Arc(s.start, ballRadius));
						
						/* Non-empty vector means ball path crosses collision arc at
						 * convex GroundSegment end
						 */
						if (pts.size()) {
							/* The collision would be the nearest of the points
							 * if there are more than one.
							 */
							vecf nearPt = pts[0];
							if (pts.size() == 2 && hyp(pts[1], oldPos) < hyp(pts[0], oldPos))
								nearPt = pts[1];
							if (ci.seg == nullptr
								|| hyp(oldPos, nearPt) < hyp(oldPos, ci.collisionPt))
								ci = CollisionInfo(&s, nearPt);
						}
					}
				}
				if (!s.concaveToNext) {
					if (hyp(s.end, path.pointPerpendicularTo(s.end)) <= ballRadius) {
						auto pts = s.lseg.intersectionPointsWith(Arc(s.end, ballRadius));
						if (pts.size()) {
							vecf nearPt = pts[0];
							if (pts.size() == 2 && hyp(pts[1], oldPos) < hyp(pts[0], oldPos))
								nearPt = pts[1];
							if (ci.seg == nullptr
								|| hyp(oldPos, nearPt) < hyp(oldPos, ci.collisionPt))
								ci = CollisionInfo(&s, nearPt);
						}
					}
				}
			} // end checking for contact with convex ends
#endif
		} // end for seg
	} // end for plat
		
	//ADD SCREEN EDGES TO VECTOR
	if (ci.seg != nullptr) { // There was a collision
		
		vecf collisionPt = ci.collisionPt;
		float oppNormal = ci.seg->oppNormal;
		float normal = ci.seg->normal;
		
		if (hole.containsCollisionPt(collisionPt)
			&& hole.approveVelocity(ball.pvelocity())) { // sophisticate
			ballInHole();
			return;
		}
		
		if (ci.seg->concaveFromPrev
			&& epsEquals(ci.collisionPt, ci.seg->checkLineIsctPrev, .01)) {
			
			collisionPt = ci.seg->checkLineIsctPrev;
			oppNormal = bisect(ci.seg->prev->oppNormal, ci.seg->oppNormal);
			normal = czdg(oppNormal + 180);
			inCrotch = true;
			// ///////DETERMINE WHICH SEG IS FROM AND TO
			crotchInfo = CrotchInfo(ci.seg, ci.seg->prev, nullptr);

//			rrr.setFillColor(Color::Black);

		}
		else if (ci.seg->concaveToNext
				 && epsEquals(ci.collisionPt, ci.seg->checkLineIsctNext, .01)) {
			collisionPt = ci.seg->checkLineIsctNext;
			oppNormal = bisect(ci.seg->oppNormal, ci.seg->next->oppNormal);
			normal = czdg(oppNormal + 180);
			inCrotch = true;
			// ///////DETERMINE WHICH SEG IS FROM AND TO (probably only important if allowing fly() to transition to roll() even when inCrotch and less than bounce clamp
			crotchInfo = CrotchInfo(ci.seg, ci.seg->next, nullptr);

			rrr.setFillColor(Color::Black); //////
		}
		auto cosValue = abs(cosd(angleBetween(oppNormal, ball.direc())));
		ball.setMag(ball.mag() * (1 - (cosValue * ci.seg->bounceLoss)));
		float normCpt = ball.mag() * cosValue;
		
		ball.sP(collisionPt);
//		if (isnan(collisionPt.x) || isnan(collisionPt.y)) { // /////////
//			ball.setColor(Color::Black);
//		}
		float fractionRemaining = (hyp(ci.collisionPt, newPos) / path.length) * pct;
		
		if (normCpt < bounceClamp && !inCrotch) { // roll/continuous contact
			//in order to transition to roll from inCrotch would probably have to go through all roll collision logic to determine which seg the ball is supposed to be "on" and which dir is going back into the crotch thus triggering 0 velocity; have to get right or the ball could appear to stick if landing in a crotch that ought to have rebounded it
			startRoll(ci.seg);
			if (!epsEquals(fractionRemaining, 0, .001))
				roll(fractionRemaining);
		}
		
		else { // bounce
			float deltaDir = 2 * angleBetween(normal, ball.oppDirec());
			float newDir = czdg(ball.oppDirec() + (clockwiseOf(ball.oppDirec(), normal) ? -deltaDir : deltaDir));
			ball.setDirec(newDir);
			string key = ci.seg->surfaceType;
			{ // kludge sound tweaks
				if ((key == "grass" || key == "rough")
					&& normCpt > 4)
					key = "grassThump";
			}
			/* Try to make the sound volume approximate the velocity of the ball */
			playSoundAtVolPct(gSound(key), normCpt / .1);
			if (!epsEquals(fractionRemaining, 0, .001))

				//??ADD GRAVITY HERE to modify dir
				
				/* Recursively call after collisions until the remaining
				 * distance for the frame is traveled
				 */
				fly(fractionRemaining);
		}
	} //end if collision

	/* No collisions, just set the ball to the next point in its trajectory */
	else {
		ball.sP(newPos);
//		if (isnan(newPos.x) || isnan(newPos.y)) { // /////////
//			ball.setColor(Color::Black);
//		}
	}
	
	/* Draw the ball's flight trajectory */
	vecf np = ci.seg ? ci.collisionPt : newPos;
	vecf pv = toPolar(np - oldPos);
	RectangleShape traj;
	traj.setSize({pv.x, 2});
	traj.setRotation(pv.y);
	traj.setPosition(oldPos);
	traj.setFillColor(withAlpha(PURPLE, 130));
	trajecRt.draw(traj);
	trajecRt.display();
} // end fly()

void State::roll(float pct, GroundSegment* segToIgnore)
{
	if (pct < .98) { rr.setFillColor(PURPLE); }// ////
	else rr.setFillColor(ORANGE);// /////

	auto oldPos = ball.gP();
	auto oldGSeg = gSeg;
	auto oldVelocity = ball.velocity;
	auto oldPVelocity = ball.pvelocity();

//	VertexArray va {Lines};
//	va.append(VTX(oldPos));
//	va.append(VTX(oldPos + pVec(ball.mag() * 30, ball.direc())));
//	if (frameCounter % 2)
//		drtDraw(va);

	/* Handle loop-like surfaces where gravity overcomes centrifugal force */
	if (angleIsOrFallsBetween(gSeg->angle, 270, 90)) {
		vecf projectedPos = oldPos + ball.velocity;
		if (gSeg->ptIsNormalFromCheckline(projectedPos)) {
			endRoll();
			
			dbgSky(Color::Red); // ////////
			
			fly(pct);
			return;
		}
	}
			 
	float normCpt = ball.mag() * absCos(gSeg->oppNormal, ball.direc());
	vecf inrPlusNorm = ball.velocity + pVec(normCpt, gSeg->normal);
	
	/* If the projected motion isn't greater than static friction,
	 * the ball doesn't move.
	 */
	if (hyp(inrPlusNorm) < gSeg->muS * normCpt) {
//	if (hyp(inrPlusNorm) < ::muS * normCpt) { // DEBUG use global so can change value
		zeroOutVelocity();
		  
//		dbgSky(PURPLE); // ////////
		  
		return;
	}
	
	if (inCrotch) {
		
//		bool movingTowardsNext = angleBetween(ball.direc(), gSeg->angle) <
//			angleBetween(ball.direc(), gSeg->oppAngle);
		bool movingTowardsCrotch =
			angleIsOrFallsBetween(ball.direc(), crotchInfo.prevSeg()->oppNormal, crotchInfo.nextSeg()->oppNormal);
		//			movingTowardsNext && crotchInfo.nextSeg() == crotchInfo.otherSeg(crotchInfo.toSeg)
//			|| !movingTowardsNext && crotchInfo.prevSeg() == crotchInfo.otherSeg(crotchInfo.toSeg);
		if (movingTowardsCrotch) {
//			dbgSky(Color::Black);
			zeroOutVelocity();
			return;
		}
	}
	
	/* There is movement: compute the kinetic friction and resulting velocity */
	inCrotch = false;
	onCusp = false;
	auto frictionCpt = pVec(gSeg->muK * normCpt, czdg(toPolar(inrPlusNorm).y + 180));
//	auto frictionCpt = pVec(::muK * normCpt, czdg(toPolar(inrPlusNorm).y + 180)); //DEBUG
		 /* Using pointPerpendicularTo instead of simply adding the normal
		  * component to the inertia and friction components so that cumulative
		  * float inaccuracies won't make the ball travel "under" the surface.
		  */
	auto newPos = gSeg->collisionCheckLine.pointPerpendicularTo(oldPos + ((ball.velocity + frictionCpt) * pct));
		  /* `path` is the projected translation this frame, but will be subsequently
		   * checked for collisions along the way
		   */
	LineSegment path {oldPos, newPos};
	  
	  /* Using `ball.velocity` to fake centrifugal force: it will not immediately
	   * line up with the translation path. Start with old velocity plus friction.
	   */
	  /* Should be safe to add frictionCpt without checks: muS should ensure
	   * that there is enough mag to subtract from.
	   */
	vecf newVelocity = ball.velocity + frictionCpt;
	vecf tempNewPos = oldPos + newVelocity;
	  // /MAY NOT WANT TO SUBTRACT GRAVITY FOR PARTIAL FRAME ROLLS
	  //OR, SUBTRACT GRAVITY BY PCT
	  
	  /* Gravity was added to `velocity` at the beginning of this frame.
	   * We want to "undo" any gravity components counter to the surface normal,
	   * but keep the rest.
	   */
	bool subtractGravityNorm = pct > .999
								&& angleIsOrFallsBetween(gSeg->angle, 90, 270)
								&& angleBetween(gSeg->normal, toPolar(newVelocity).y) > 90;
	if (subtractGravityNorm) {
		float cptMag = absCos(90, gSeg->oppNormal) * gravity;
		auto perpPt = gSeg->collisionCheckLine.pointPerpendicularTo(tempNewPos);
		newVelocity += pVec(min(cptMag, hyp(perpPt, tempNewPos)), gSeg->normal);
		tempNewPos = oldPos + newVelocity;
	}
	  
	float tempNewDir = toPolar(newVelocity).y;
	float xlatDir = angleBetween(tempNewDir, gSeg->angle) <
				  angleBetween(tempNewDir, gSeg->oppAngle) ?
				  gSeg->angle : gSeg->oppAngle;
	  
	if (!gSeg->ptIsNormalFromCheckline(tempNewPos)) {
		  /* Incrementally moving ball velocity towards parallel
		   * with surface by adding a normal vector proportional to cur friction,
		   * to fake the diminishing of centrifugal force.
		   */
		float mag = hyp(frictionCpt) * centrifugalDecmFactor;
		  /* Only add this component if new velocity is far enough from parallel
		   * to surface.
		   */
		if (mag < gSeg->collisionCheckLine.perpDistanceTo(tempNewPos))
			  newVelocity += pVec(mag, gSeg->normal);
		  
		  /* Else "centrifugal force" has dissipated, and we're rolling right
		   * in line with the surface.
		   */
		else newVelocity = toRect(absCos(tempNewDir, xlatDir) * hyp(newVelocity), xlatDir);
	}
	  
	  /* Regardless of the results of collision checking, this is the new
	   * velocity we want to treat the ball as traveling with.
	   */
	ball.setVelocity(newVelocity);
	float xlatMag = absCos(xlatDir, ball.direc()) * ball.mag();
	
	if (xlatMag < speedClamp) //NEW, TEST
		zeroOutVelocity(); //
	else disableShooting();   //
	
	/* Prepare for collision checks with other surfaces */
	CollisionInfo ci;
	for (auto& p : platforms) {
		  
		auto rec = p.va.getBounds();
		rec = FloatRect(rec.left - 8, rec.top - 8, rec.width + 16, rec.height + 16);
		if (!path.intersectsWith(rec)
			&& !rec.contains(oldPos)
			&& !rec.contains(newPos))
			continue;
		  
		for (auto& s : p.segs) {
			  
			  /* Don't check the seg the ball is on */
			if (&s == gSeg
				  || &s == segToIgnore)
				continue;
			  
			  /* Have to approach this block differently from fly() because
			   * ball.direc() represents the direction the ball is "trying"
			   * to go, and provides centrifugal force as the ball passes
			   * from seg to seg. However, the ball is translating parallel
			   * with the angle of gSeg, so that's what we have to use for
			   * checking valid collision direction.
			   */
			bool validCollisionDir = angleIsOrFallsBetween(xlatDir, czdg(s.angle + 180), s.angle);
			if (!validCollisionDir)
				continue;
			  
			vecf isctPt;
			if (path.intersectsWith(s.collisionCheckLine, &isctPt)
				  && ptHasValidNumbers(isctPt)) {
				if (ci.seg == nullptr
					  || hyp(oldPos, isctPt) < hyp(oldPos, ci.collisionPt))
					ci = CollisionInfo(&s, isctPt);
			}
//			else { //roll into convex corner? (in "extra code.txt")
//			}
		} // end for seg
	} // end for plat
	  
		
	  /* Check whether the ball is traveling off of gSeg over a convex corner.
	   * Store it if it occurs sooner than any other potential collisions
	   * with the path. Previous block ignored checklines that were approached
	   * from the "back" side, so convex transitions weren't considered.
	   */
	bool passedNext = hyp(gSeg->checkLineIsctPrev, newPos) > gSeg->collisionCheckLine.length;
	bool passedPrev = hyp(gSeg->checkLineIsctNext, newPos) > gSeg->collisionCheckLine.length;
	vecf isctPt;
	if (epsEquals(xlatDir, gSeg->angle, 1) && !gSeg->concaveToNext) {
		if (
//				  path.intersectsWith(gSeg->next->collisionCheckLine.line, &isctPt)
//				  && ptHasValidNumbers(isctPt)
			  passedNext
			  ) {
			isctPt = gSeg->checkLineIsctNext;
			if (ci.seg == nullptr
				|| hyp(oldPos, isctPt) < hyp(oldPos, ci.collisionPt))
				ci = CollisionInfo(gSeg->next, isctPt, true);
		}
	}
	else if (epsEquals(xlatDir, gSeg->oppAngle, 1) && !gSeg->concaveFromPrev) {
		if (
//			  path.intersectsWith(gSeg->prev->collisionCheckLine.line, &isctPt)
//			  && ptHasValidNumbers(isctPt)
				  passedPrev
			  ) {
			isctPt = gSeg->checkLineIsctPrev;
			if (ci.seg == nullptr
				|| hyp(oldPos, isctPt) < hyp(oldPos, ci.collisionPt))
				ci = CollisionInfo(gSeg->prev, isctPt, true);
		}
	}
	
	if (!ci.seg) {
		
		bool almostIsctsNext =
			!passedNext
			&& epsEquals(xlatDir, gSeg->angle, 1)
			&& epsEquals(newPos, gSeg->checkLineIsctNext, snapToEndEps);
		bool almostIsctsPrev =
			!passedPrev
			&& epsEquals(xlatDir, gSeg->oppAngle, 1)
			&& epsEquals(newPos, gSeg->checkLineIsctPrev, snapToEndEps);
		
		//EXTRA HANDLING TO MAKE SURE frame doesn't end on junction pt?
		//if fracrem eps 0 (or ball could reach vel 0,0), back the ball outside of snap eps
		if (passedNext
			|| almostIsctsNext
//				&& gSeg->concaveToNext // // not snapping for convex
			) {
			ci = CollisionInfo(gSeg->next, gSeg->checkLineIsctNext, !gSeg->concaveToNext);
			if (!passedNext) // only if fracRem > 0?
				newPos = gSeg->checkLineIsctNext;
			dbgSky(PEACH); // ///////
				//SEGTOIGNORE? LOOK AHEAD FOR xlatDIR CHANGE?
		}
		else if (passedPrev
				 || almostIsctsPrev
//					&& gSeg->concaveFromPrev
				 ) {
			ci = CollisionInfo(gSeg->prev, gSeg->checkLineIsctPrev, !gSeg->concaveFromPrev);
			if (!passedPrev)
				newPos = gSeg->checkLineIsctPrev;
			dbgSky(PEACH); // ///////
		}
	}
	  

	/* Ball in hole? */
	if (path.intersectsWith(rectWithAddedMarginOf(hole.gGB(), 5))
		//first check jump velocity and handle before approving
		&& hole.approveVelocity(ball.pvelocity())) {
		ballInHole();
		return;
	}


	  /* There is either a collision or a passing of a convex segment end */
	if (ci.seg != nullptr) {
		GroundSegment& seg {*(ci.seg)};
		vecf collisionPt {ci.collisionPt};

		  /* In any of the following cases, set the ball at the point where
		   * collision happened or terrain changed; remainder of this frame's
		   * trajectory will be recomputed and completed subsequently.
		   */
		ball.sP(collisionPt);

		/* Get the percentage of the total frame travel still to be
		 * carried out.
		 */
		float fractionRemaining = epsEquals(path.length, 0, .0001) ? 0 : (hyp(collisionPt, newPos) / path.length) * pct;
		 
		  /* Handling rolling past convex ends */
		if (ci.passingConvexEnd) {
			
			onCusp = true;
			 // /SHOULD THIS USE OLD VELOCITY INSTEAD
			vecf projectedPos = collisionPt + ball.velocity;
			 /* Don't simulate a roll if: Either old or new segment has "upside
			  * downness." Do if the projected position lies "under" the new segment's
			  * checkline. The projected position is "above" the new segment's
			  * checkline by less than a specified epsilon.
			  */
			bool doConvexRoll = angleIsOrFallsBetween(gSeg->angle, 90, 270)
								&& angleIsOrFallsBetween(ci.seg->angle, 90, 270)
								&& angleBetween(gSeg->angle, ci.seg->angle) < maxAngForRollCvx
								&& (!ci.seg->ptIsNormalFromCheckline(projectedPos)
									|| ci.seg->collisionCheckLine.perpDistanceTo(projectedPos) < convexRollClamp);
			 
			 /* Ball cresting a convex surface but keeping continuous
			  * contact with ground
			  */
			if (doConvexRoll) {
				 
				dbgSky(Color::Yellow); // ////////////
				 
				 /* Keep the magnitude of previously computed new velocity
				  * but change the direction to align with new segment.
				  */
				ball.setVelocityP(xlatMag, epsEquals(xlatDir, gSeg->angle, 1) ? ci.seg->angle : ci.seg->oppAngle);
				gSeg = ci.seg;
				if (!epsEquals(fractionRemaining, 0, fracRemEps))
					roll(fractionRemaining, oldGSeg);
			}
			 
			 /* Ball catching air off convex surface */
			else {
				endRoll();
				 
				dbgSky(ORANGE); // ////////////
				 
				ball.setVelocityP(xlatMag, xlatDir);
				if (!epsEquals(fractionRemaining, 0, fracRemEps))
					fly(fractionRemaining);
			}
		}
		  
		  /* Handle collisions/transitions with concave surfaces */
		else {
			 
			inCrotch = true;
			 /* Ball traversing a concave transition gentle enough to
			  * simulate curvature/continuous roll
			  */
			if (angleBetween(gSeg->angle, seg.angle) < maxAngForRoll) {

				 /* For centrifugal, capture the relative deviation of ball's
				  * velocity from last seg, and apply the same deviation in
				  * relation to the newly entered seg.
				  */
				float ang = clockwiseAngleBetween(gSeg->angle, ball.direc());
				ball.setDirec(czdg(ci.seg->angle + ang));
				 
				 /* Add a centrifugal component representing the extra force
				  * from trying to "plow" into a steeper slope.
				  */
				 /* IF CONTINUING TO USE, .24 FOR FACTOR IS ONLY VALUE THAT
				  * behaved approximately as desired
				  */
				float centrifMag = absSin(gSeg->angle, ci.seg->angle) * xlatMag;
				ball.addVelocityP(centrifMag * centrifugalIncmFactor, ci.seg->oppNormal);
				 
				crotchInfo = CrotchInfo(gSeg, ci.seg, ci.seg);
				gSeg = ci.seg;
				 
				dbgSky(Color::Magenta); // /////////////

				if (!epsEquals(fractionRemaining, 0, fracRemEps))
					roll(fractionRemaining);
			}
			 
			 /* Ball contacting another surface and rebounding */
			else {
				 /* Decrement magnitude of velocity by appropriate bounce loss */
				auto cosValue = absCos(seg.oppNormal, xlatDir);
				ball.setMag(xlatMag * (1 - (cosValue * seg.bounceLoss)));
				 
				 /* Get rebound direction */
				float oppDirec = czdg(xlatDir + 180);
				float deltaDir = 2 * angleBetween(seg.normal, oppDirec);
				float newDir = czdg(oppDirec + (clockwiseOf(oppDirec, seg.normal) ? -deltaDir : deltaDir));
				 
				 /* Tentatively locate ball's position in next frame */
				 
				 // / INCOMPLETE if adding wind, other forces
				 
				vecf projectedPos = ball.gP() + (pVec(ball.mag(), newDir)
//													  + vGravity
												  )
//														* fractionRemaining
				 ;
//					 drtDraw(DbgPoint(projectedPos));
//					 drtDraw(DbgPoint(ball.gP() + (pVec(ball.mag(), newDir)
//													+ vGravity
//												   ) * fractionRemaining));
				bool reboundGetsAir =
					gSeg->ptIsNormalFromCheckline(projectedPos)
					&& gSeg->collisionCheckLine.perpDistanceTo(projectedPos) > bounceClamp;
				 
				 /* Angle of rebound will cause ball to fly */
				if (reboundGetsAir) {
					 
					dbgSky(Color::Blue); // /////////////

					 // /TRYING FRACTIONAL GRAVITY HERE; not confident this ends
					 //up at same place as projectedPos. Would this call fall
					 //through if fracRem is less than 1 and full gravity is
					 //added next frame before ball moves
					ball.setDirec(newDir
//										  + vGravity * fractionRemaining
									  );
					 //this crInfo is probably never used: already should
					 //have confirmed that ball will fly next frame
					crotchInfo = CrotchInfo(gSeg, ci.seg, nullptr);
					endRoll();
					 
					if (!epsEquals(fractionRemaining, 0, fracRemEps))
						fly(fractionRemaining);
				}
				 
				else {
					if (gSeg->facesUp
						 && angleIsOrFallsBetween(ball.direc(), 0, 180)
						 && !isCloserToHorizontal(gSeg->angle, ci.seg->angle)) {
						 
						float ang = clockwiseAngleBetween(gSeg->angle, ball.direc());
						ball.setDirec(czdg(ci.seg->angle + ang));
						 
						dbgSky(Color::Cyan); // /////////////

						crotchInfo = CrotchInfo(gSeg, ci.seg, ci.seg);
						gSeg = ci.seg;
						if (!epsEquals(fractionRemaining, 0, fracRemEps))
							roll(fractionRemaining, oldGSeg);
					}
					 
					 /* Ball will continue rolling but in reversed direction */
					else {
							
						dbgSky(Color::Green); // /////////////

						 // /THIS COULD BE insufficient if ball is upside down,
						 //or discrepancies should be caught by reboundGetsAir?
						ball.setDirec(czdg(xlatDir + 180));
//						drtDraw(collisionPt, vecf(100, xlatDir), Color::Red);
//						drtDraw(collisionPt, vecf(100, ball.direc()), Color::Blue);
						 
						 /* If rolling against a "wall" that angles back towards
						  * the "ground", reduce speed according to angle, on
						  * top of the reduction already taken from  bounce loss.
						  */
						 /* Using ball.mag() instead of xlatMag because mag has already
						  * been recomputed due to bounce loss, and no centrifugal force
						  * is in play for the new ball.mag()
						  */
//							 if (angleBetween(gSeg->angle, seg.angle) > 90) {
							 float sinValue = abs(sind(angleBetween(seg.oppNormal, xlatDir)));// ///////////
							 ball.setMag(ball.mag() - (sinValue * ball.mag()));
//							 }
						crotchInfo = CrotchInfo(gSeg, ci.seg, gSeg);
					 
						if (!epsEquals(fractionRemaining, 0, fracRemEps))
							roll(fractionRemaining);
					} // end ball rolls 180deg from old dir
				} // end !rebound gets air
				
				playSoundAtVolPct(gSound(ci.seg->surfaceType), xlatMag / .1f);
				
			} // end rebounding
		} // end collision/transition with concave
		if (epsEquals(fractionRemaining, 0, fracRemEps)
			&& onCusp && gSeg
			// inCrotch?
			) {
			ball.move(pVec(snapToEndEps + .008,
						   //not guaranteed to always move away from cusp?
						   angleBetween(ball.direc(), gSeg->angle) < 90 ? gSeg->angle : gSeg->oppAngle
						   ));
		}
	} // end path interrupted by concave or convex change
	  
	  /* No collisions or changes in the terrain: use the originally projected
	   * path for new ball location at end of frame.
	   */
	else {
		ball.sP(newPos);
//		if ( //DBG CHECK
//			hyp(gSeg->checkLineIsctPrev, newPos) > gSeg->collisionCheckLine.length
//			|| hyp(gSeg->checkLineIsctNext, newPos) > gSeg->collisionCheckLine.length
//			  ) { // /////////
//				  dbgSky(Color::Black);
//		}
	}

	  //HANDLE EDGES OF SCREEN //////////////
	{
		auto rightBound = curCourse->curHole->viewSize.x;
		auto bottomBound = curCourse->curHole->viewSize.y;
		auto newPos = ball.gP();
		if (newPos.x < 2 || newPos.x > rightBound - 2) {
			ball.sP(newPos.x < 0 ? 1.5 : rightBound - 2.5, ball.gP().y);
			ball.setDx(ball.dx() * -1);
//			ball.setMag(ball.mag() * 1.3);
			
			//seems to have fixed the skyrocketing but it can also cause the next fly collision check to be just shy and falls through the platform
//			if (angleIsOrFallsBetween(gSeg->oppAngle, 270, 359.9) && ball.dx() < 0
//				|| angleIsOrFallsBetween(gSeg->oppAngle, 0, 90) && ball.dx() > 0) {
//				rolling = false;
//				gSeg = nullptr;
//				ball.setDy(ball.dy() * 1.2);
//			}
		}
		if (newPos.y > bottomBound) {
			ball.sP(ball.gP().x, bottomBound - 1);
			zeroOutVelocity();
		}
	}
	// //////////////////
}

void State::endRoll()
{
	rolling = false;
	gSeg = nullptr;
	disableShooting();
}

void State::zeroOutVelocity ()
{
	ball.setVelocity(0,0);
	if (!teeingOff)
		startNewShotTimer();
}

void State::disableShooting ()
{
	if (!teeingOff) {
		timedMgr->removeByTag("setCanShoot");
		timedMgr->gUnset("canShoot");
	}
}

void State::startNewShotTimer ()
{
	if (!timedMgr->gOn("canShoot"))
		timedMgr->addEventIf("setCanShoot", 1.2, [&]() {
			timedMgr->gSet("canShoot");
			trajecRt.draw(deh);
			trajecRt.display();
		});
}

void State::fadeTrajectory ()
{
	Image img {trajecRt.getTexture().copyToImage()};
	forNum(scrh) {
		forNumJ(scrw) {
			/* Get pixel from the trajectory rentex */
			auto pix = img.getPixel(j, i);
			/* If it's transparent (not drawn to last frame) get the
			 * pixel from the stored image and decrease its alpha
			 */
			if (pix.a == 0) {
				pix = trajecImg.getPixel(j, i);
				if (pix.a == 0)
					continue;
				else (pix.a = max(0, pix.a - 5));
			}
			trajecImg.setPixel(j, i, pix);
		}
	}
	trajecRt.clear(Color::Transparent);
	trajecTx.update(trajecImg);

/*
 // THIS LEAVES faint trajectory behind and taints the sky color
 Color c = gw->redrawColor;
 c.a = 5;
 RectangleShape r;
 r.setSize({scrw, scrh});
 r.setFillColor(c);
 drtDraw(r);
 */

/*
 //causes purple color to degenerate to blackish before fading
 ZImage zim {drt.getTexture().copyToImage()};
 zim.fadeByAlphaVal(5); //turn off if all ghosts are faded
 Texture tx;
 tx.loadFromImage(zim);
 Sprite spr(tx);
 drt.clear(Color::Transparent);
 drtDraw(spr);
 */
}

void State::ballInHole ()
{
	ballActive = false;
	endRoll();
	zeroOutVelocity();
	inCrotch = false;
	onCusp = false;
	ball.sP(hole.ballLoc());
	gSound("ballInHole").play();
	timedMgr->addEvent(1.2, [&]() { gSound("cheer").play();});
	timedMgr->addEvent(5, [&]() {
		loadNextHole();	});
}



vector<pair<string, string>> State::fillTypeList
{
	{ "rockwall.png", "rockwall" },
	{ "woodpostfill.png", "woodpostfill" },
	{ "marblefill.png", "marblefill" },
	{ "brickfill.png", "brickfill" },
	{ "dirtfill.png", "dirtfill" },
	{ "cratefill.png", "cratefill" }
};

vector<pair<string, string>> State::surfaceTypeList
{
	/* Unless modifying logic in assembleSprite for fillInFromImage,
	 * keep keys the same as the extensionless filename
	 */
	{ "grass.png", "grass" },
	{ "dirt.png", "dirt" },
	{ "brick.png", "brick" },
	{ "sand.png", "sand" },
	{ "marble.png", "marble" },
	{ "metal.png", "metal" },
	{ "puttgrass.png", "puttgrass" },
	{ "logs.png", "logs" },
	{ "stones.png", "stones" },
	{ "rough.png", "rough" }
};

map<string, SurfacePhysics> State::physicsMap
{
	{ "grass", {"grass", .1, .1, .45, 20}}
	, { "puttgrass", {"puttgrass", .06, .06, .38, 20}}
	, { "rough", {"rough", .3, .4, .65, 18}}
	, { "sand", {"sand", .4, .5, .85, 12}}
	, { "dirt", {"dirt", .35, .45, .75, 14}}
	, { "marble", {"marble", .01, .01, .13, 23}}
	, { "metal", {"metal", .012, .012, .18, 22}}
	, { "stones", {"stones", .03, .03, .25, 21}}
	, { "logs", {"logs", .03, .03, .27, 21}}
	, { "brick", {"brick", .03, .03, .25, 21}}
};


void State::dbgSky(Color c)
{
#ifdef DBG
	app->setRedrawColor(c);
#endif
}
