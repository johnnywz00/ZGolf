
#include "sfmlApp.hpp"


SFGameWindow::SFGameWindow () {
	
	setup(defaultTitle, vecU(defaultWidth, defaultHeight));
}

//SFGameWindow::SFGameWindow (const string& title, const vecU& size) {
//	
//	setup(title, size);
//}
	
void SFGameWindow::setup (const string& title, const vecU& size) {
	
	windowTitle = title;
//	windowSize = size;
	_isFullscreen = true;
//	_isFullscreen = false;
	_isStretched = false;
	_isDone = false;
	_isFocused = true;
	create();
//	if (!loadByMethod(icon, iconPath)) {
	if (!icon.loadFromFile(iconPath)) {
		cerr << "Couldn\'t load icon! \n";
	}
	else
		window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
	}

void SFGameWindow::create () {
	
    VideoMode mode = VideoMode::getDesktopMode();
	auto style = Style::Default;
	
	if (_isFullscreen) {
		auto fsmodes = VideoMode::getFullscreenModes();
		if (fsmodes.size()) {
			mode = fsmodes[0];
			style = Style::Fullscreen;
		}
		else _isFullscreen = false;
	}
	window.create(mode, windowTitle, style);
	View v { window.getDefaultView() };
	windowSize = vecU(v.getSize().x, v.getSize().y);
	SCRW = windowSize.x;
	SCRH = windowSize.y;
	SCRCX = SCRW / 2;
	SCRCY = SCRH / 2;
	screenOffsetFrom1440x900 = vecf(-(v.getSize().x - 1440) / 2,
									-(v.getSize().y - 900) / 2);
	window.setFramerateLimit(60);
//	setToggledView(_isStretched);
}
	
void SFGameWindow::toggleFullscreen () {
	
	_isFullscreen = !_isFullscreen;
	destroy();
	create();
}

void SFGameWindow::toggleStretchGraphics () {
	
	_isStretched = !_isStretched;
	setToggledView(_isStretched);
}

void SFGameWindow::setToggledView (bool stretched) {
	
	View v { FloatRect(0, 0, 1440, 900) };
	if (!stretched) {
		v = window.getDefaultView();
//		v.move(screenOffsetFrom1440x900);
	}
	window.setView(v);
}



Game::Game () :
		window() {
			
    srand(unsigned(time(nullptr)));
    state.w = window.getRenderWindow();
	state.gw = &window;
	state.timedMgr = &timedMgr;
    state.onCreate();
    clock.restart();
}

void Game::update () {
	
	state.mxOld = state.mx;
	state.myOld = state.my;
    Event event;
    while (window.window.pollEvent(event)) {
		vecf adj = window.window.mapPixelToCoords(vecI(event.mouseButton.x,
													   event.mouseButton.y));
		if (!state.handleTextEvent(event)) {
			
			switch(event.type) {
					
				case Event::KeyPressed:
					switch(event.key.code) {
						case Keyboard::F5:
							window.toggleFullscreen();
							break;
						case Keyboard::F6:
							window.toggleStretchGraphics();
							break;
						default:
							state.onKeyPress(event.key.code);
							break;
					}
					break;
					
				case Event::KeyReleased:
					switch(event.key.code) {
						default:
							state.onKeyRelease(event.key.code);
							break;
					}
					break;
					
				case Event::MouseMoved:
					adj = window.window.mapPixelToCoords(vecI(event.mouseMove.x, event.mouseMove.y));
					state.mx = int(adj.x);
					state.my = int(adj.y);
					break;
					
				case Event::MouseButtonPressed:
					state.onMouseDown(int(adj.x), int(adj.y));
					break;
					
				case Event::MouseButtonReleased:
					state.onMouseUp(int(adj.x), int(adj.y));
					break;
					
				case Event::Closed:
					window.close();
					break;
					
				case Event::LostFocus:
					window._isFocused = false;
					break;
					
				case Event::GainedFocus:
					window._isFocused = true;
					break;
					
				default:
					break;
			}
		}
    }
    state.update(elapsed);
}



int main () {
	
        /* XCode folly: two instances of the program are launched if
		 * we customize the working directory. We can cause the extraneous
         * instance to silently quit immediately by giving it a relative
         * path that it can't find.
         */
    Image img;
	if (!img.loadFromFile(iconPath))
        return EXIT_FAILURE;
    

    Game game;
    while (!game.getWindow()->isDone()) {
        game.update();
        game.render();
        game.lateUpdate();
    }
}
