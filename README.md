Overview
========

This is my ever changing 2D SDL + OpenGL game engine that I've used to create
several small games, mainly for various [Warwick Game Design](http://www.warwickgamedesign.co.uk/) competitions.

Current features
================

+ Sprite texture atlases.
+ Bitmapped-font rendering.
+ Automatic caching of sounds + textures.
+ Efficient 2D sprite batching.
	- The whole scene is drawn in a single glDrawElements per texture.
+ Game state management.
	- Menus can be opened while still drawing stuff underneath e.t.c.
+ Builds with gcc or mingw32.

In-development features.
========================

+ ARB shaders.
+ In-game console + console variables.
+ Other stuff!

