# Brawl - Mass Brawl!!!

![Brawl](brawl.png)

Brawl is a game about intense action with super realistic physics. You have to lead your gang to victory in a crime-infected town.

You have to improve your fighting skills continuously, learn to block attacks, punch or kick with the correct timing. Watch your health and energy state. You can pick up fainted opponents/friends and use guns.

FEATURES

- Intense action  
- Fluid animation  
- 6 different scenes  
- Responsive controls  
- Dozens of buddies and enemies  

The code is also suitable for learning purposes.

## Try it! ##

[https://github.com/milgra/brawl/releases/download/0.8/brawl.app.tar.gz]

Extract the archive and call brawl.app/brawl. It is a standalone application bundle running on all distributions.

### Files

audio.c - audio handling
bus.c - Message bus for inter-module communication  
cmd.c - Command queue   
defaults.c - Settings and defaults  
file.c - Config file handling  
image.c - Image loading  
main.c - SDL handling, top level event handling  
pivot.c - Pivot points in the scene  
scene.c - Scene handling
scenerenderer.c - Scene renderer
settings.c - Settings
str_util.c - String utils
view.c - View handler

actor - actor handling
gl - OpenGL handling
math - math functions

### Tech info

Brawl is a pure C project written in [headerless C](https://github.com/milgra/headerlessc).  
It uses reference-counted memory management ( zc_memory.c ).  
It uses modular programming with mediator - implementation groups with different functionality that don't include each other directly, they send messages to each other.  
If a module is "static" a global variable is used to store it's state, instancing is meaningless .  

### WebAssembly 

find src -type f -name "*.c" > files.txt

 /home/milgra/Downloads/emsdk/upstream/emscripten/emcc --embed-file ./res -Isrc/brawl -Isrc/zc_core -Isrc/zc_math -Isrc/zc_gfx -Isrc/brawl/gl -Isrc/brawl/math -Isrc/brawl/actor -Isrc/ui -Isrc/ext -I/home/milgra/Downloads/emsdk/upstream/emscripten/system/includer/emscripten.h -DPATH_MAX=255 -DPKG_DATADIR=\"/\" -DBRAWL_VERSION=\"1.0\" -sUSE_SDL=2 -sUSE_SDL_MIXER=2 -sUSE_FREETYPE=1 -sMAX_WEBGL_VERSION=1 -sALLOW_MEMORY_GROWTH=1 $(cat files.txt) -o brawl.html

### TODO

- update physics from cljs-brawl
- imageelement throw error is resource directory not exists braw/cortex
- blood not visible
- re-write physics and collosion based on cljs-brawl  
- add snow and other new stuff from cljs-brawl  
- zc_util2 should show its a math lib 
- labfej tapadjon foldre  
- ne sebezzen a hulla amig viszed  
- rombolhato tereptargyak  
- intersection detection egyszerusitese
- after first win multiply enemy count
