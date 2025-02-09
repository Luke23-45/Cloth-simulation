#define SDL_MAIN_HANDLED
#include <iostream>
#include <SDL2/SDL.h>
#include <vector>
#include <cmath>
#include <algorithm>    

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int POINT_RADIUS = 5;
const int SPRING_ORIGINAL_LENGTH = 100;
const double GRAVITY = .5;
const double SPRING_STRENGTH = 0.08;
const double SPRING_LENGTH = 100.0;
const double DAMPING = .05;

bool isDragging = false;

// Point struct
struct Point {
    double x;
    double y;
    double vx;
    double vy;
    bool isFixed;
    Point(int a, int b, bool c) : x(a), y(b), isFixed(c), vx(0), vy(0) {}
};

struct Spring {
    int p1;
    int p2;
    Spring(int a, int b) : p1(a), p2(b) {}
};


// Storage container for points and springs
std::vector<Point*> points;
std::vector<Spring*> springs;

template<typename T>
T clamp(T value, T min, T max) {
    return (value < min) ? min : (value > max) ? max : value;
}

bool initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("2D Spring Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

void destroyScreen() {
    // Free allocated memory for points and springs
    for (auto point : points) {
        delete point;
    }
    for (auto spring : springs) {
        delete spring;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// Calculating the forces
void calculateForces() {
    for (auto& point : points) {
        if (!point->isFixed) {
            point->vy += GRAVITY;
        }
    }
    for (const auto& spring : springs) {
        Point* p1 = points[spring->p1];
        Point* p2 = points[spring->p2];

        double dx = p2->x - p1->x;
        double dy = p2->y - p1->y;
        double distance = std::hypot(dx, dy);
        double displacement = SPRING_ORIGINAL_LENGTH - distance;

        if (true) {
            double force = SPRING_STRENGTH * displacement;
            double fx = (dx / distance) * force;
            double fy = (dy / distance) * force;

            if (!p1->isFixed) {
                p1->vx -= fx - DAMPING * (p1->vx);
                p1->vy -= fy - DAMPING * (p1->vy);
            }
            if (!p2->isFixed) {
                p2->vx += fx - DAMPING * (p2->vx);
                p2->vy += fy - DAMPING * (p2->vy);
            }
        }
    }
}

void updatePoints() {
    for (auto& point : points) {
        if (!point->isFixed) {
            point->x += point->vx;
            point->y += point->vy;

            if (point->x - POINT_RADIUS <= 0 || point->x + POINT_RADIUS >= SCREEN_WIDTH) {
                point->vx = -point->vx;
                point->x = clamp(point->x, (double)POINT_RADIUS, (double)(SCREEN_WIDTH - POINT_RADIUS));
            }

            if (point->y - POINT_RADIUS <= 0 || point->y + POINT_RADIUS >= SCREEN_HEIGHT) {
                point->vy = -point->vy;
                point->y = clamp(point->y, (double)POINT_RADIUS, (double)(SCREEN_HEIGHT - POINT_RADIUS));
            }
        }
    }
}

void addPoint(double x, double y, bool isFixed = false) {
    points.push_back(new Point(x, y, isFixed));
}

void addSpring(int a, int b) {
    springs.push_back(new Spring(a, b));
}

void renderPoints() {
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    for (const auto& point : points) {
        SDL_Rect rect = {(int)(point->x - POINT_RADIUS), (int)(point->y - POINT_RADIUS), POINT_RADIUS * 2, POINT_RADIUS * 2};
        SDL_RenderFillRect(renderer, &rect);
    }
}

void renderSprings() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (const auto& spring : springs) {
        SDL_RenderDrawLine(renderer, (int)(points[spring->p1]->x), (int)(points[spring->p1]->y), (int)(points[spring->p2]->x), (int)(points[spring->p2]->y));
    }
}

void renderScene() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    renderPoints();
    renderSprings();
    SDL_RenderPresent(renderer);
}

void addPointAndSpringOnClick(int x, int y, bool isFixed = false) {
    addPoint(x, y, isFixed);
    if (points.size() > 1) {
        int lastIndex = points.size() - 2;  // Second to last index
        int newIndex = points.size() - 1;  // Last index (new point)
        addSpring(lastIndex, newIndex);
    }
}

void removeLastPointAndSpring() {
    if (!springs.empty()) {
        delete springs.back(); 
        springs.pop_back();
    }
    if (!points.empty()) {
      if(points.size() > 1){
        delete points.back(); 
        points.pop_back();
      }

    }
}

void pushNearbyPoints(int mouseX, int mouseY, double pushDistance) {
    for (auto& point : points) {
        if (point->isFixed) continue; // Skip fixed points

        double dx = point->x - mouseX;
        double dy = point->y - mouseY;
        double distance = std::sqrt(dx * dx + dy * dy);

        if (distance < 50.0) { 
            double factor = pushDistance / distance; 
            point->vx += dx * factor;
            point->vy += dy * factor;
        }
    }
}

int main() {
    if (!initSDL()) {
        return 1;
    }

    // Create points and springs
    addPoint(400, 100, true);  // Fixed point
    addPoint(400, 200);
    addPoint(500, 200);
    addSpring(0, 1);
    addSpring(1, 2);

    SDL_Event e;
    bool quit = false;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                }
                if (e.key.keysym.sym == SDLK_r) {
                    removeLastPointAndSpring();
                }
            }
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_RIGHT) {
                int mouseX = e.button.x;
                int mouseY = e.button.y;
                pushNearbyPoints(mouseX, mouseY, 35.0); // Push points by a fixed distance of 10.0
            }
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int mouseX = e.button.x;
                int mouseY = e.button.y;
                addPointAndSpringOnClick(mouseX, mouseY);
            }
        }

        calculateForces();
        updatePoints();
        renderScene();
        SDL_Delay(16);
    }
    destroyScreen();
    return 0;
}