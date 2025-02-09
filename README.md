# Cloth Simulation

This project implements a simple cloth simulation using SDL2 for rendering. The simulation models the cloth as a grid of interconnected points, with springs connecting adjacent points. It simulates the physics of the cloth by applying forces like gravity and spring tension, and includes features like wind interaction and the ability to tear the cloth.

## Getting Started

### Prerequisites
Ensure you have the following installed:
- C++ Compiler (e.g., g++)
*   [SDL2](https://www.libsdl.org/)
*   [SDL2_ttf](https://www.libsdl.org/projects/SDL_ttf/) (Not necessary)

## Project Structure

*   `cloth_sim.cc`: Main C++ source file.
*   `Makefile`: Build the project.
*   `src/`: Source code directory.
    *   `include/`: Header files.
    *   `lib/`: SDL related

### Installation
1. Clone the repository:
   ```bash
   git clone git@github.com:Luke23-45/Cloth-simulation.git
   ```
## Building

This project uses `make` for building. To build the project, run the following command in your terminal:

2. Navigate to the project directory:
   ```bash
   cd Cloth-simulation
   ```
3. Compile the code:
   ```bash
    make
   ```
4. Run the executable:
   ```bash
   ./spring_simulation
   
   ```
5. In window:
   ```bash
   main.exe
   ```
6. To clean up the build artifacts
   ```bash
    make clean
   ```

## Features

* **Physics Simulation:**  The cloth's movement is governed by basic physics principles, including gravity, spring forces, and friction.
* **Spring Model:** The cloth is represented as a network of points connected by springs.  The simulation calculates the forces exerted by these springs based on their extension or compression.
* **Point-Based Interaction:**  Users can interact with the cloth by:
    * **Left-clicking:** Applying a "wind" force that affects points near the mouse cursor.
    * **Right-clicking:** Tearing the cloth by removing springs near the mouse cursor and deleting any resulting orphaned (unconnected) points or disconnected components.
* **Boundary Constraints:**  The cloth is constrained to stay within the window boundaries.
* **Fixed Points:**  Some points of the cloth can be designated as "fixed," meaning they are not affected by the simulation forces. This allows for anchoring the cloth.

## Key Controls

| **Action**                 | **Key/Mouse**      |
| -------------------------- | ------------------ |
| Wind Effect                | Left mouse button  |
| Simulate tearing the cloth | Right mouse button |
| Exit simulation            | `ESC` key          |

## Code Structure

- **Point Structure:**  
    Represents a node in the cloth grid. Each `Point` stores its current and previous positions (to enable Verlet integration), a flag (`isFixed`) to indicate if it is immovable, and a counter (`num`) for the number of attached springs.
    
- **Spring Structure:**  
    Represents a connection between two `Point` objects, enforcing the distance constraint that simulates cloth tension.
    
- **Core Functions:**
    
    - `createCloth()`:  
        Initializes the cloth by creating a grid of points and connecting adjacent points with springs. Certain points (typically along the top edge) are fixed to simulate attachment.
        
    - `sdlInit()`:  
        Initializes SDL, creates the window and renderer.
        
    - `renderCloth()`:  
        Renders all springs as lines and points as small red squares.
        
    - `applyConstraints()`:  
        Keeps each point within the screen boundaries.
        
    - `calculateForce()`:  
        Applies spring forces to adjust point positions based on the deviation from the natural spring length.
        
    - `windEffect()`:  
        Applies an external force (simulating wind) to points near a mouse click.
        
    - `removeSpringAndUpdatePoints()`:  
        Removes springs near the mouse click (right-click) and decrements the spring count for connected points.
        
    - `cleanupOrphanedPoints()`:  
        Deletes any non-fixed points that have lost all their spring connections.
        
    - `removeDisconnectedComponents()`:  
        Uses a breadth-first search (BFS) to find and remove points that become disconnected from any fixed point.
        
    - `updatePoints()`:  
        Updates the position of each point based on physics.
        
    - `destroyScreen()`:  
        Cleans up resources by freeing memory and shutting down SDL.
        

## Demo Video
Check out the project demo video on YouTube: https://www.youtube.com/watch?v=McOcbGHyAWA
## License

This project is licensed under the MIT License. Feel free to use, modify, and distribute the code.

## Acknowledgements

- SDL2 for graphics rendering.
- This simulation draws inspiration from various cloth simulation techniques and the principles of spring-mass systems based on classical mechanics.
