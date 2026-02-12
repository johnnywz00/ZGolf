# ZGolf

## C++/SFML 2D platformer golf game

<img width="1728" height="1117" alt="Screenshot 2026-02-12 at 4 58 06 PM" src="https://github.com/user-attachments/assets/23c4faed-b039-4cfe-b22a-12f822cf847e" />

### ABOUT THE PROJECT

## [COMING SOON (ish?)]

### FILE DESCRIPTIONS
* **sfmlApp:**  Implements `main()` and the abstract app
* **state:**  Implements primary graphical elements; physics; game logic
  
(From my "reusable modules" repo: https://github.com/johnnywz00/SFML-shared-headers)
* **jwz:**  C++ utility functions, #defines, shortcuts
* **jwzsfml:**  Like above, but SFML-specific

### BUILDING INSTRUCTIONS
Ready-made program files are available on the Releases page of this repository, with versions for MacOS, Windows, and Linux. NO INSTALLATION NECESSARY: just download and double-click. If your OS isn't supported by the pre-made versions, or if you have other reasons for building from source:
- Clone this repository, and navigate to the root folder of that clone in a terminal window.
- Run:
<pre>
   cmake -B build
   cmake --build build
</pre>
