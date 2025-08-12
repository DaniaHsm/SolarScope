# COMP 371 Project - SolarScope: Solar System Simulation

Welcome to SolarScope! An interactive 3D solar system simulation that brings astronomy education to life. This project allows users to explore planetary sizes and orbital mechanics, observe realistic shadow effects between celestial bodies, and experience extreme gravitational phenomena through our black hole simulation. Read the following guide to learn how to navigate and interact with our solar system. 

### **Libraries Used:**
- glfw 3.4
- glew 2.2.0
- glm 1.0.1
- assimp 6.0.2

### **Key Features & Educational Value:**
**Interactive Learning Tools:**
- **Realistic Solar System**: Complete solar system with all planets and celestial bodies
- **Planet Selection Mode**: Focus on individual planets with detailed information panels  
- **Size Comparison Mode**: Align planets by size to compare their relative scales
- **Shadow Observation**: Watch how planets cast realistic shadows on each other
- **Orbital Mechanics Study**: Learn planetary speeds and distances through interactive controls

**Creative & Scientific Effects:**
- **Time Control**: Speed up, slow down, or reverse time to observe orbital patterns
- **Black Hole Simulation**: Experience gravitational effects as planets get consumed within a black hole
- **Comet Trails**: Beautiful elliptical orbits with dynamic particle trails
- **Celestial Glow**: Atmospheric lighting effects around each planet

## Controls:

### Camera Movement:
- **1 & 2**: Switch between first-person and third-person view, as a rubber duck object circulating space!!
- **W A S D**: Move around the solar system
- **Mouse or Arrow Keys**: Look around/rotate camera
- **Shift**: Move faster through space

### Planet Selection Mode:
- **3**: Enter planet selection mode / Cycle through planets
- **4**: Exit planet selection mode (return to free camera)
- **I**: Show/hide detailed information panel for selected planet

### Time Control:
- **Space**: Pause/unpause time (all orbital movements)
- **9**: Reverse time direction (planets orbit backwards)
- **0**: Reset orbital speed to normal (1x speed)
- **+ (Plus)**: Increase orbital speed (up to 10x faster)
- **- (Minus)**: Decrease orbital speed (can go into reverse)

### Special Modes:
- **X**: Trigger black hole effect 
- **R**: Reset world to normal state (after black hole)
- **C**: Activate size comparison mode (align planets by size)

### Advanced Features:
- **Combine modes**: Use planet selection while in comparison mode for detailed planet study
- **Time manipulation**: Speed up or reverse time in any mode to see orbital patterns

## Educational Value:
- Learn relative planet sizes and orbital distances
- Observe how planets cast shadows on each other
- Study orbital mechanics with time control
- Compare planetary characteristics with detailed info panels
- Understand gravitational effects through black hole simulation

## Project Requirements Compliance:

### Assignment 1 Requirements
- **Interactive Camera**: Mouse/keyboard controls with rotation and translation relative to facing direction
- **Multiple Textured Surfaces**: All planets, sun, moon, and comet use unique textures
- **Hierarchical Animation**: 2+ levels deep - planets orbit sun, moon orbits Earth (orrery system)
- **Library Compliance**: Uses only OpenGL, GLFW, GLM, GLEW, stb_image.h, Assimp, C++ standard library

### Assignment 2 Requirements 
- **Dynamic Lighting with Phong Model**: Sun acts as moving light source with realistic Phong lighting
- **Complex 3D Models**: Duck model loaded via Assimp from GLTF file format
- **Dynamic Shadows**: Planets cast realistic shadows on each other based on sun position
- **Interactive Elements**: Planet selection, time control, black hole effects, comparison mode
- **Advanced Camera System**: Multiple camera modes including planet tracking and free fly

### **Technical Implementation:**
- **Lighting System**: Phong lighting model with sun as primary light source
- **Shadow Rendering**: Real-time shadow calculations between celestial bodies  
- **File Loading**: Assimp integration for complex 3D model loading (duck.gltf)
- **Texture Management**: 10+ unique planetary textures with proper UV mapping
- **Animation System**: Hierarchical transformations for orbital mechanics
- **Shader Programs**: Multiple specialized shaders (celestial, skybox, UI, model rendering)

### **Advanced Features Beyond Requirements:**
-  **Black Hole Physics**: Gravitational collapse with realistic visual effects
-  **Time Manipulation**: Variable speed controls including complete time reversal  
-  **Educational Info Panels**: Interactive planet data with custom texture overlays
-  **Elliptical Comet Orbits**: Realistic comet physics with dynamic particle trails
-  **Multi-Mode Integration**: Seamlessly combine different viewing modes
-  **Cosmic Environment**: 360° space skybox with atmospheric effects

## Contributors:
- Yusuf Chahal
- Dania Houssami
- Mohammad Alshikh
- Ava Samimi
