#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cmath>

#define WIDTH 800
#define HEIGHT 800
#define NUM_BOIDS 50
#define MOVE_SPEED 3
#define NEIGHBOURS_DIST 50
#define SEPARATION_DIST 15
#define SEPARATION_FORCE 5

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

float normalize_angle(float angle) {
    return angle - 2 * M_PI * floor((angle + M_PI) / (2 * M_PI));
}

float calc_distance(int this_boid, int other_boid) {
    float dist_x = boids[other_boid].x - boids[this_boid].x;
    float dist_y = boids[other_boid].y - boids[this_boid].y;
    return sqrt(dist_x * dist_x + dist_y * dist_y);
}

void separation(int this_boid, float* dx, float* dy) {
    *dx = 0, *dy = 0;
    for (int boid = 0; boid < NUM_BOIDS; boid++) {
        float dist_x = boids[boid].x - boids[this_boid].x;
        float dist_y = boids[boid].y - boids[this_boid].y;
        if (boid == this_boid) continue;
        if (calc_distance(this_boid, boid) < SEPARATION_DIST) {
            *dx += (dist_x / calc_distance(this_boid, boid)) * SEPARATION_FORCE;
            *dy += (dist_y / calc_distance(this_boid, boid)) * SEPARATION_FORCE;
        }
    }
}

void alignment(int this_boid, float* heading) {
    float avg_heading = 0, count = 1;
    for (int boid = 0; boid < NUM_BOIDS; boid++) {
        if (boid == this_boid) continue;
        if (calc_distance(this_boid, boid) < NEIGHBOURS_DIST) {
            avg_heading += boids[boid].angle;
            count++;
        }
    }

    *heading = avg_heading / count;
    
}

void update_boids() {
    for (int boid = 0; boid < NUM_BOIDS; boid++) {
        float dx = 0, dy = 0, heading = 0;
        separation(boid, &dx, &dy);

        if (dx != 0 || dy != 0) {
            float heading = atan2(dy, dx);
            float diff = heading - boids[boid].angle;
            boids[boid].angle += normalize_angle(diff) * 0.1f;
        }

        alignment(boid, &heading);
        if (heading != 0) {
            float diff = heading - boids[boid].angle;
            boids[boid].angle += normalize_angle(diff) * 0.1f;
        }

        float theta = boids[boid].angle;
        boids[boid].x += MOVE_SPEED * cos(theta);
        boids[boid].y += MOVE_SPEED * sin(theta);

        // Boundary conditions
        if (boids[boid].x >= WIDTH) {
            boids[boid].x = 5;
        } else if (boids[boid].x <= 0) {
            boids[boid].x = WIDTH-5;
        }

        if (boids[boid].y >= HEIGHT) {
            boids[boid].y = 5;
        } else if (boids[boid].y <= 0) {
            boids[boid].y = HEIGHT-5;
        }

        boids[boid].angle += ((rand() % 3 - 1) * 0.1f);
        if (boid == 10) {
            SDL_Log("X: %f, Y: %f, Angle: %f", boids[boid].x, boids[boid].y, boids[boid].angle);
        }
    }
}

void draw_boids() {
    const int BOID_SIZE = 10;  // Adjust this value to change boid size
    SDL_Vertex vertices[3];  // Three vertices for each triangle
    
    // Set common vertex properties
    for (int boid = 0; boid < NUM_BOIDS; boid++) {
        for (int i = 0; i < 3; i++) {
            if (boid == 10) {
                vertices[i].color.r = 1.0f;  // Red
                vertices[i].color.g = 0.0f;
                vertices[i].color.b = 0.0f;
                vertices[i].color.a = 1.0f;
            } else {
                vertices[i].color.r = 1.0f;  // White
                vertices[i].color.g = 1.0f;
                vertices[i].color.b = 1.0f;
                vertices[i].color.a = 1.0f;
            }
            vertices[i].tex_coord.x = 0;
            vertices[i].tex_coord.y = 0;
        }

        // Front point (nose of the triangle)
        vertices[0].position.x = boids[boid].x + BOID_SIZE * cos(boids[boid].angle);
        vertices[0].position.y = boids[boid].y + BOID_SIZE * sin(boids[boid].angle);
        
        // Back points with wider spread
        const float backAngle = 2.5f;  // Controls triangle width
        vertices[1].position.x = boids[boid].x + (BOID_SIZE * 0.5f) * cos(boids[boid].angle + backAngle);
        vertices[1].position.y = boids[boid].y + (BOID_SIZE * 0.5f) * sin(boids[boid].angle + backAngle);
        
        vertices[2].position.x = boids[boid].x + (BOID_SIZE * 0.5f) * cos(boids[boid].angle - backAngle);
        vertices[2].position.y = boids[boid].y + (BOID_SIZE * 0.5f) * sin(boids[boid].angle - backAngle);
        
        // Draw the filled triangle
        SDL_RenderGeometry(renderer, NULL, vertices, 3, NULL, 0);
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