# OpenRPG

Cross-platform RPG Maker XP/VX inspired 2D game engine, primarily designed to be used as a
base API for other languages to create bindings for.

The base library is implemented in pure C for maximum performance and interopablity. The
library does all the heavy lifting, allowing any language that binds to it to focus on
gameplay and design, without being concerned over low-level rendering, audio, input, timing, etc.

## Implemented

(All object types meet and exceed the basic functionality/implementation found in RPG Maker series)

- [x] Base "Game" class, that is a combination of the RPG Maker "Graphics" module and an interface to the application
- [x] Audio (OpenAL backend), with audio effects, "fire-and-forget" sounds, many audio formats
- [x] Input (keyboard and mouse)(gamepad support is planned for the future)
- [x] Image/Bitmap, many supported image formats
- [x] Sprite (tone, color, blending, etc)
- [x] Viewport
- [x] Plane
- [x] Font (TrueType format) with proper kerning
- [ ] Tilemap (in progress). Uses [TMX](https://doc.mapeditor.org/en/stable/reference/tmx-map-format/) format used by by [Tiled](https://www.mapeditor.org/)
- [x] Basic shader functions exposed, GLSL format
- [x] Built-in, feature rich "scene transition" implementation that uses shaders, and traditional RPG Maker images
