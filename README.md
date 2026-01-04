# GameEngine


A 2D game engine built with C++ and SDL2, using an Entity Component System (ECS) architecture.\
This project is intended as a technical demo and learning exercise, not a full commercial game.

[Watch the demo video](https://www.youtube.com/watch?v=Go6RMoumZYc&feature=youtu.be)

## Features

### Core
- Entity Component System architecture (see [ARCHITECTURE.md](ARCHITECTURE.md))
- Singleton-based managers (Renderer, Audio, Input, Assets)
- Delta-time based game loop
- Object pooling for entities and resources
- Binary serialization/deserialization for save data

### Graphics
- SDL2 hardware-accelerated rendering
- TGA and PNG texture loading
- Sprite sheets with configurable clip sizes
- Parallax scrolling backgrounds with configurable depth factors
- Camera with entity following and world/screen coordinate conversion
- Logical resolution scaling (resolution-independent rendering)

### Maps
- Tiled (.tmx) map loading via tmxlite
- Infinite scrolling with procedural chunk system
- Chunk types: start, random, gap, floating platforms
- Tile collision detection
- Object layers for spawn points and collision shapes

### Physics & Gameplay
- Gravity with ground detection
- Jump with adjustable height (hold to jump higher)
- Coyote time (grace period after leaving platform)
- Dash ability with cooldown
- Punch attack with range detection
- Patrol AI for enemies
- Health system with invincibility frames
- Collectibles with point values
- Spatial partitioning grid for O(n) collision detection
- Two-phase collision: broad-phase AABB + narrow-phase detailed checks

### Audio
- 16-channel sound effect mixing
- Background music with play/pause/stop
- Channel completion callbacks

### Input
- Keyboard key down/up detection
- Mouse position and button states
- Game controller support
- Text input handling

### UI
- TTF font rendering
- UI states: Start Screen, Playing, Paused, Game Over
- Clickable buttons with hover states
- Animated heart health display
- Score display
- Keyboard shortcuts for menu navigation

## Requirements

- Visual Studio 2019+
- Windows x64

## Dependencies (Included)

- SDL2 2.26.1
- SDL2_image 2.8.8
- SDL2_mixer 2.8.1
- SDL2_ttf 2.20.1
- GLM
- tmxlite

## Building

1. Open `GameEngine.sln` in Visual Studio
2. Select `x64` and `Debug` or `Release`
3. Build (F7)
4. Run from `x64/Debug/` or `x64/Release/`

## Project Structure

```
GameEngine/
├── Core/       - GameController, Timing, Singleton
├── Game/       - Entity, Components, Systems, ChunkMap, GameUI, SpatialGrid, Level, Unit
├── Graphics/   - Renderer, Camera, TileMap, AnimatedSpriteLoader, Texture
├── Audio/      - AudioController, Song, SoundEffect
├── Input/      - InputController, Keyboard, Mouse, Controller
├── Resources/  - AssetController, Resource, Serializable
├── Utils/      - ObjectPool, StackAllocator
└── Assets/
    ├── Textures/   - Player, Enemy, Obstacle sprites
    ├── Maps/
    │   ├── Chunk/  - TMX map chunks
    │   └── Images/ - Tilesets
    └── Fonts/      - TTF fonts

README.md           - This file
ARCHITECTURE.md     - ECS architecture documentation
```

## Controls

| Key | Action |
|-----|--------|
| A/D | Move |
| Space | Jump (hold for higher) |
| Shift | Dash |
| Left Click | Punch |
| Escape | Pause / Quit |

## License

See individual library licenses in `External/`.