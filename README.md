# ZGolf

## C++/SFML 2D platform golf game

<img width="1728" height="1117" alt="Screenshot 2026-02-12 at 4 58 06 PM" src="https://github.com/user-attachments/assets/23c4faed-b039-4cfe-b22a-12f822cf847e" />

### ABOUT THE PROJECT
ZGolf is a project that was a kind of natural outcome of carrying previous experiments to the next level. It integrates the projectile physics of Parasheep, the sphere collisions of LetterBalls, and the sprite collision checking of Hopscotch, and adds new levels of sophistication to the whole. Even so, the first incarnation of this game came relatively early in my programming career. Years later, the physics got a major overhaul, and the editor gained a lot more functionality. No engines or game libraries are used beyond SFML, which handles fundamentals like drawing to screen, playing sound, and getting input events. 

The game's platforms are made with curving terrain which dictates how the ball bounces or rolls. There are different surface types (deep grass, short grass, sand...), which dictate how far the ball will roll, how much bounce will be absorbed, etc. New holes can be created in an editor, and sets of holes can be grouped together into courses. 

**CODE REVIEWER NOTE:** This game is still in a state of evolution, even if active development is sporadic at best: as a result, the codebase retains a healthy dose of debug blocks, temporary measures, and inelegant implementations left over from early days (when the game didn't know where it was headed) that haven't yet mustered the priority to be redone.

Certain features warrant a disclaimer until I have time to reimplement; among them:
<details>
<summary>The list...</summary>

* Play mode and Design mode should be separate game states with their own members and methods
* The limited animation was a quick and dirty measure for the kids' sake (marked as @kludgeAnim in the code): a more robust and generalized animation system is in the plans
* The physics work pretty well (and the casual tester/player may come away without noticing anything amiss,) but I haven't gotten to the end of troubleshooting them, and depending on the terrain I create, it is still possible to trigger

	* stack overflow
	* ball "falling" through the ground
	* ball "freezing" in a crotch between two ground segments
	* ball disappearing on account of being assigned a position of INFINITY or NAN
	* ball "creeping" when it is supposed to always come to rest by reason of static friction (probably because of the faux centrifugal force, see below)
	
* The Line classes need to be reconfigured to calculate with respect to y instead of x when lines have a slope greater than 1, to avoid inaccuracies caused by enormous slope values when lines are close to vertical. 
* I'm still in the experimentation phase of how to model centrifugal force, re: allowing putting around a loop. It's possible that I just need to keep tweaking the numerical values I'm using, but it's also possible that my current approach isn't going to produce wholly satisfactory results. I'm trying to model the ball "sticking" to an upside down surface when it has enough momentum, but it has been causing the ball to unrealistically "dig in" too hard and slow down elsewhere. 
</details>

The disclaimers aside, it has been a really good project for providing enjoyable challenges of logic and implementation. I had never done any research (purposefully, as most of my personal projects were meant to be problem-solving exercises for myself) on how other games modeled a ball interacting with curved terrain,
<details>
<summary>... and more babble about that</summary>

 so the entire design approach was a blank slate. Ultimately, I settled on a system of straight but short ground segments to approximate a curve. This necessitated keeping track of angles between neighboring segments, to determine (sometimes based on the ball's velocity at the time) whether the ball should "bounce" off the change in angle (or catch air, if convex), or whether it should simulate a smooth roll from one to the next. This much I was able to accomplish without academia-level math and calculus, although I did enjoy learning the geometric process of constructing a spline from two control points, and figuring how to put it to code. In the editor, you can place a series of points; there can be 0, 1 or 2 control points between each pair of points, and this draws a pure curving spline that represents a new section of terrain to be added to the level. The points and controls can be dragged around for adjustment, and when finalized, an algorithm creates ground segments at but no less than a small length threshold that follow the spline drawn in the editor. From there, each segment has definite bounds and an angle, which information can be used in the bouncing and rolling logic. No doubt pure math and calculus could be used, but I envision the formulae necessary to define whimsical curves going willy-nilly everywhere to be very complex and unwieldy. 

In playing mode, most of the interesting logic resides in the fly() and roll() methods. 

The stone wall generator (shown in the game screenshot above) was a whole other algorithm challenge that I had set for myself independently of this game, and then incorporated here. 

The splash/menu screen background came from a generator I made as another self-challenge to implement Delaunay triangulation and Voronoi tessellation.
</details>

Not surprisingly, I have a pretty long wish list of additional hopes and features for the game, but it's tough to say if or when time will allow. Wind, water hazards, levels that are larger than the screen size and can pan the view, more animations, better graphical details and corresponding editor functions.

The little teddy player character has autobiographical roots, and is explained a little bit in the README of my Hopscotch repository.

### FILE DESCRIPTIONS
* **sfmlApp:**  Implements `main()` and the abstract app
* **zgolf:**  State class; contains the primary game mechanics
* **designMethods:**  More State methods that deal with editor (should become own class)
* **GroundSegment:**  The workhorse class for all ball-physics interactions
* **ToolWindow:**  Class for the draggable editor tool selection pane
* **platforms:**  Play version and Design version of a Platform: collection of GroundSegments with some other data
* **Vert:**  An interactive point or control point used to "sketch" the curving outline of a new platform in the editor
* **objects:**  Misc. structs: Course, CourseHole, CollisionInfo, etc.
  
(From my "reusable modules" repo: https://github.com/johnnywz00/SFML-shared-headers)
* **jwz:**  C++ utility functions, #defines, shortcuts
* **jwzsfml:**  Like above, but SFML-specific
* **zsprite:**  Wrapper class for SFML sprite with many extra methods, particularly related to collision checking
* **vsprite:**  Subclass of ZSprite that uses velocity
* **resourcemanager:**  Static class for accessing resource files globally
* **timedeventmanager:**  Manages fuses/daemons, delayed callbacks

### BUILDING INSTRUCTIONS
Ready-made program files are available on the Releases page of this repository, with versions for MacOS, Windows, and Linux. NO INSTALLATION NECESSARY: just download and double-click. If your OS isn't supported by the pre-made versions, or if you have other reasons for building from source:
- Clone this repository, and navigate to the root folder of that clone in a terminal window.
- Run:
<pre>
   cmake -B build
   cmake --build build --parallel
</pre>
