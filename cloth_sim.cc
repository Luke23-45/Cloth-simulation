#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <unordered_set>
#include <queue>

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int GRID_SIZE = 50;
const double SPRING_LENGTH = 12;
const int POINT_RADIUS = 1;
const double GRAVITY = 0.01;
const double FRICTION = 0.999;

//Represents a position in the cloth simulation.
struct Point {
    double x, y, oldX, oldY;
    bool isFixed;
    int num;  // How many springs are attached to this point.
    
    Point(double a, double b, bool isfix)
      : x(a), y(b), oldX(a), oldY(b), isFixed(isfix), num(0) {}
      
    void update() {
        if (isFixed)
            return;
        double vx = (x - oldX) * FRICTION;
        double vy = (y - oldY) * FRICTION + GRAVITY;
        oldX = x;
        oldY = y;
        x += vx;
        y += vy;
    }
};


// Spring structure: holds direct pointers to the connected points.

struct Spring {
    Point* p1;
    Point* p2;
    
    Spring(Point* a, Point* b) : p1(a), p2(b) {}
};

std::vector<Point*> points;
std::vector<Spring*> springs;


// createCloth()
//  - Creates a grid of points (with some points fixed)
//  - Creates springs between adjacent points (horizontal and vertical)
//  - Increments the 'num' field in each point to keep track of how many springs are //attached.

void createCloth(){
    bool isFix = false;
    // Create all points
    for (int y = 0; y < GRID_SIZE; y++){
        for (int x = 0; x < GRID_SIZE; x++){
            isFix = (y == 0 && (x % 5 == 0 || (x == GRID_SIZE - 1)));
            if (y == 0 && x == 19) {  // last point.
                isFix = true;
            }
            // Offset the grid position to display in the window.
            points.push_back(new Point(x * SPRING_LENGTH + 100, y * SPRING_LENGTH + 50, isFix));
        }
    }
    
    // Create springs between adjacent points.
    for (int y = 0; y < GRID_SIZE; y++){
        for (int x = 0; x < GRID_SIZE; x++){
            int currentIndex = y * GRID_SIZE + x;
            // Horizontal connection
            if (x > 0) {
                int leftIndex = y * GRID_SIZE + (x - 1);
                springs.push_back(new Spring(points[currentIndex], points[leftIndex]));
                points[currentIndex]->num++;
                points[leftIndex]->num++;
            }
            // Vertical connection
            if (y > 0) {
                int aboveIndex = (y - 1) * GRID_SIZE + x;
                springs.push_back(new Spring(points[currentIndex], points[aboveIndex]));
                points[currentIndex]->num++;
                points[aboveIndex]->num++;
            }
        }
    }
}


// sdlInit()
//  - Initializes SDL, creates a window and a renderer.

bool sdlInit(){
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "Error: " << SDL_GetError() << "\n";
        return false;
    }
    window = SDL_CreateWindow("Cloth Simulation", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT,
                              SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Error: " << SDL_GetError() << "\n";
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "Error: " << SDL_GetError() << "\n";
        return false;
    }
    return true;
}

// renderCloth()
//  - Renders all springs as lines and all points as small red squares.

void renderCloth(){
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (const auto& spring : springs) {
        SDL_RenderDrawLine(renderer,
                           (int)(spring->p1->x), (int)(spring->p1->y),
                           (int)(spring->p2->x), (int)(spring->p2->y));
    }
    
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (const auto& point : points) {
        SDL_Rect rect = { (int)(point->x - POINT_RADIUS),
                          (int)(point->y - POINT_RADIUS),
                          POINT_RADIUS * 2, POINT_RADIUS * 2 };
        SDL_RenderFillRect(renderer, &rect);
    }
}


// applyConstraints()
//  - Constrains each point to remain within the screen bounds.

void applyConstraints(){
    for (auto& point : points){
        if (point->isFixed)
            continue;
        if (point->x <= 0)
            point->x = 0;
        if (point->x + POINT_RADIUS * 2 >= SCREEN_WIDTH)
            point->x = SCREEN_WIDTH - POINT_RADIUS * 2;
        if (point->y <= 0)
            point->y = 0;
        if (point->y + POINT_RADIUS * 2 >= SCREEN_HEIGHT)
            point->y = SCREEN_HEIGHT - POINT_RADIUS * 2;
    }
}


// calculateForce()
//  - Adjusts the positions of points based on spring tension.

void calculateForce(){
    for (auto& spring : springs){
        double dx = spring->p2->x - spring->p1->x;
        double dy = spring->p2->y - spring->p1->y;
        double dist = std::sqrt(dx * dx + dy * dy);
        double mag = (dist - SPRING_LENGTH) / dist;
        
        if (!spring->p1->isFixed) {
            spring->p1->x += 0.5 * dx * mag;
            spring->p1->y += 0.5 * dy * mag;
        }
        if (!spring->p2->isFixed) {
            spring->p2->x -= 0.5 * dx * mag;
            spring->p2->y -= 0.5 * dy * mag;
        }
    }
}


// renderScreen()
//  - Clears the screen and then renders the cloth.

void renderScreen(){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    renderCloth();
    SDL_RenderPresent(renderer);
}


// windEffect()
//  - Applies a force to points within a 100-pixel radius of the mouse click.

void windEffect(int mx, int my){
    for (auto& point : points) {
        double dx = point->x - mx;
        double dy = point->y - my;
        double dist = std::sqrt(dx * dx + dy * dy);
        if (dist < 100) {
            point->oldX -= dx * 0.4;
            point->oldY -= dy * 0.4;
        }
    }
}


// removeSpringAndUpdatePoints()
//  - Checks each spring to see if either of its endpoints is near the mouse click.
//  - If so, decrements the attached spring count for the two connected points and removes the spring.

void removeSpringAndUpdatePoints(double mx, double my) {
    for (auto springIt = springs.begin(); springIt != springs.end(); ) {
        Spring* spring = *springIt;
        
        double dx1 = spring->p1->x - mx;
        double dy1 = spring->p1->y - my;
        double dist1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
        
        double dx2 = spring->p2->x - mx;
        double dy2 = spring->p2->y - my;
        double dist2 = std::sqrt(dx2 * dx2 + dy2 * dy2);
        
        if (dist1 < 15 || dist2 < 15) {
            // Decrement the spring count in both points.
            spring->p1->num--;
            spring->p2->num--;
            delete spring;
            springIt = springs.erase(springIt);  // removal of spring.
        }
        else {
            ++springIt;
        }
    }
}


// cleanupOrphanedPoints()
//  - Removes any nonfixed point that has no springs attached.

void cleanupOrphanedPoints() {
    auto newEnd = std::remove_if(points.begin(), points.end(),
        [](Point* p) {
            if (!p->isFixed && p->num <= 0) {
                delete p;
                return true;
            }
            return false;
        });
    points.erase(newEnd, points.end());
}


// updatePoints()
//  - Calls the update method on all points.

void updatePoints(){
    for (auto& point : points) {
        point->update();
    }
}


// destroyScreen()
//  - Frees all allocated memory and shuts down SDL.

void destroyScreen(){
    // Delete remaining springs.
    for (auto& spring : springs) {
        delete spring;
    }
    springs.clear();
    // Delete remaining points.
    for (auto& point : points) {
        delete point;
    }
    points.clear();
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void removeDisconnectedComponents() {
  if (points.empty()) return;

  std::unordered_set<Point*> visited;
  std::queue<Point*> queue;

  // Collect all fixed points as starting points
  for (auto& point : points) {
      if (point->isFixed) {
          queue.push(point);
          visited.insert(point);
      }
  }

  // BFS traversal to find all connected points
  while (!queue.empty()) {
      Point* p = queue.front();
      queue.pop();

      // Check all springs to find neighbors
      for (auto& spring : springs) {
          if (spring->p1 == p || spring->p2 == p) {
              Point* neighbor = (spring->p1 == p) ? spring->p2 : spring->p1;
              if (!visited.count(neighbor)) {
                  visited.insert(neighbor);
                  queue.push(neighbor);
              }
          }
      }
  }

  // Collect non-fixed points not visited (disconnected)
  std::vector<Point*> pointsToRemove;
  for (auto& point : points) {
      if (!point->isFixed && !visited.count(point)) {
          pointsToRemove.push_back(point);
      }
  }

  if (pointsToRemove.empty()) return;

  // Create a set for faster lookup
  std::unordered_set<Point*> pointsToRemoveSet(pointsToRemove.begin(), pointsToRemove.end());

  // Collect springs connected to points to remove
  std::unordered_set<Spring*> springsToRemove;
  for (auto& spring : springs) {
      if (pointsToRemoveSet.count(spring->p1) || pointsToRemoveSet.count(spring->p2)) {
          springsToRemove.insert(spring);
      }
  }

  // Decrement 'num' for the connected points of each spring to remove
  for (auto* spring : springsToRemove) {
      spring->p1->num--;
      spring->p2->num--;
  }

  // Remove springs from the vector and delete them
  springs.erase(std::remove_if(springs.begin(), springs.end(),
      [&springsToRemove](Spring* s) { return springsToRemove.count(s); }),
      springs.end());

  for (auto* spring : springsToRemove) {
      delete spring;
  }

  // Remove points from the vector and delete them
  points.erase(std::remove_if(points.begin(), points.end(),
      [&pointsToRemoveSet](Point* p) { return pointsToRemoveSet.count(p); }),
      points.end());

  for (auto* point : pointsToRemove) {
      delete point;
  }
}


// main()
//  - Runs the simulation loop, handling events, updating physics, and rendering.

int main(){
    if (!sdlInit()){
        return 1;
    }
    
    createCloth();
    
    SDL_Event e;
    bool quit = false;
    Uint32 startFrame, frameTime;
    
    while (!quit) {
        startFrame = SDL_GetTicks();
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                quit = true;
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE)
                    quit = true;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    // Left-click: apply wind effect.
                    windEffect(e.button.x, e.button.y);
                }
                if (e.button.button == SDL_BUTTON_RIGHT) {
                    // Right-click: remove springs near the mouse click,
                    // then remove any points that become orphaned.
                    removeSpringAndUpdatePoints(e.button.x, e.button.y);
                    cleanupOrphanedPoints();
                    removeDisconnectedComponents(); 
                }
            }
        }
        
        updatePoints();
        calculateForce();
        applyConstraints();
        renderScreen();
        
        frameTime = SDL_GetTicks() - startFrame;
        if (frameTime < 16)
            SDL_Delay(16 - frameTime);
    }
    
    destroyScreen();
    return 0;
}
