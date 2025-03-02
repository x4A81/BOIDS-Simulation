#include <SDL3/SDL.h>
#include <cmath>

#define WIDTH 800
#define HEIGHT 800
#define NUM_BOIDS 50
#define MOVE_SPEED 3

SDL_Window* window;
SDL_Renderer* renderer;

typedef struct t_boid {
    float x, y;
    float angle;
} t_boid;

t_boid boids[50];

void init_boids() {
    for (int boid = 0; boid < NUM_BOIDS; boid++) {
        boids[boid].x = rand() % WIDTH;
        boids[boid].y = rand() % HEIGHT;
        boids[boid].angle = (float)(rand()) / RAND_MAX * 2.0f * M_PI;
    }
}

void update_boids() {
    for (int boid = 0; boid < NUM_BOIDS; boid++) {
        float theta = boids[boid].angle;
        boids[boid].x += MOVE_SPEED * cos(theta);
        boids[boid].y += MOVE_SPEED * sin(theta);

        // Boundary conditions
        int turn_around = 0;
        if (boids[boid].x >= WIDTH) {
            boids[boid].x = WIDTH;
            turn_around = 1;
        }
        
        if (boids[boid].x <= 0) {
            boids[boid].x = 0;
            turn_around = 1;
        }

        if (boids[boid].y >= HEIGHT) {
            boids[boid].y = HEIGHT;
            turn_around = 1;
        }

        if (boids[boid].y <= 0) {
            boids[boid].y = 0;
            turn_around = 1;
        }

        if (turn_around) {
            boids[boid].angle += M_PI;
        }
        
        boids[boid].angle += ((rand() % 3 - 1) * 0.1f);
    }
}

void draw_boids() {
    for (int boid = 0; boid < NUM_BOIDS; boid++) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderPoint(renderer, boids[boid].x, boids[boid].y);
    }
}

int main() {
    // Setup
    if (SDL_Init(SDL_INIT_VIDEO) == 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow("Boids Simulation", WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    renderer = SDL_CreateRenderer(window, "opengl");
    if (!renderer) {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    init_boids();
    int running = 1;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();
    int frameCount = 0;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        // Update boids
        update_boids();

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw the boids
        draw_boids();

        SDL_RenderPresent(renderer);
        
        // Calculate FPS
        frameCount++;
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastTime >= 1000) {
            float fps = frameCount / ((currentTime - lastTime) / 1000.0f);
            SDL_Log("FPS: %.2f", fps);
            frameCount = 0;
            lastTime = currentTime;
        }

    }
    
    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}