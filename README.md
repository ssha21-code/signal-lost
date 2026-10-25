# signal-lost
A grid-based 2D game featuring tunnels, rooms and different entities.
* You can configure the number of hearts, rooms, and the map size in the title screen.
* The Player is a detective, and spawns in the safe main room with no dangers.
* The Player can light up only a small circle around them.
* The Player can have 3-10 hearts, based on the configuration.
* The Player can move with WASD keys.
* The Player can explore other rooms using corridors.
* The rooms may be booby-trapped with spike traps, that deal 1 heart of damage.
* The rooms may also have hearts, which the Player can collect by standing on it, and increments the hearts count by 1, up till the maximum allowed number of hearts.
* The rooms may also have ghosts spawning in them, which can shoot at the Player when the Player enters the room. The ghost's attack does 1 heart of damage to Player.
* The Player can dodge the ghost's attack by moving away, and can also shoot at the ghost by aiming with mouse pointer and pressing space.
* The main objective of the Player is to collect all the orbs, which lay scattered across all rooms.
* Arrows pointing to the direction of the orbs appear on the screen.
* Orbs also give out a small light around them.
* The tab key can be pressed to toggle debug mode, which highlights all entities around the map.
* The repository also has a builtin version of the game, where all the media such as fonts, audio, and images are built into the executable file.
* Compiling command: "g++ "file.cpp" -std=c++17 -g -static -static-libgcc -static-libstdc++ -o "file.exe" -I "raylib-5.5_win64_mingw-w64\include" -L "raylib-5.5_win64_mingw-w64\lib" -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows"
