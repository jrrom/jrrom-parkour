#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <raylib.h>
#include <rcamera.h>

void DrawLobby(Model);
void UpdateMouseCamera(Camera *);

// ===============================================================
// Starting player values
// ===============================================================

typedef struct Player {
    Vector3 position;
    float speed;
    float sensitivity;
    float fovy;
    bool levels[5];
} Player;

Player player = { (Vector3){ 0.0f, 2.0f, 0.0f }, 0.16f, 10.0f, 90.0f, { false, false, false, false, false } };

int main(void) {
    // ===============================================================
    // Init
    // ===============================================================
    
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "jrrom parkour");
    SetTargetFPS(60);
    
    // ===============================================================
    // Creating a camera
    // ===============================================================
    
    Camera camera = { 0 };
    camera.position = player.position;
    camera.target = (Vector3){ 10.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = player.fovy;
    camera.projection = CAMERA_PERSPECTIVE;

    // ===============================================================
    // Loading Up Models
    // ===============================================================
    
    Model signboard = LoadModel("resources/Signboard.glb");

    // ===============================================================
    // Limit cursor to relative movement
    // ===============================================================
    
    DisableCursor();

    // ===============================================================
    // Main Loop
    // ===============================================================

    while (!WindowShouldClose())
    {
        // Update Camera
        UpdateMouseCamera(&camera);
        
        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
            DrawLobby(signboard);
            EndMode3D();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}

// ===============================================================
// Helper Functions
// ===============================================================

void UpdateMouseCamera(Camera *camera) {
    // ===============================================================
    // To make the camera follow the mouse
    // ===============================================================

    Vector2 delta = GetMouseDelta();
    delta.x *= player.sensitivity * DEG2RAD;
    delta.y *= player.sensitivity * DEG2RAD;

    Vector3 movement = {0};

    // ===============================================================
    // Movement
    // ===============================================================
    
    bool in_strafe = false;
    if (
        (IsKeyDown(KEY_W) || IsKeyDown(KEY_S))
        &&
        (IsKeyDown(KEY_D) || IsKeyDown(KEY_A))
    ) in_strafe = true;
    float strafe_multiplier = in_strafe ? 1 / sqrt(2.0f) : 1;
    

    if (IsKeyDown(KEY_W)) {
        movement.x += player.speed * strafe_multiplier;
    }

    if (IsKeyDown(KEY_S)) {
        movement.x -= player.speed * strafe_multiplier;
    }

    if (IsKeyDown(KEY_D)) {
        movement.y += player.speed * strafe_multiplier;
    }

    if (IsKeyDown(KEY_A)) {
        movement.y -= player.speed * strafe_multiplier;
    }

    // ===============================================================
    // Camera Updation
    // ===============================================================
    
    UpdateCameraPro(camera, movement, (Vector3){delta.x, delta.y, 0.0f}, 0.0f);

    // Update player position
    player.position = camera->position;
}

void DrawLobby(Model signboard) {
    DrawCube((Vector3){ 0.0f, -1.0f, 0.0f}, 20.0f, 2.0f, 20.0f, GRAY);
    DrawGrid(300, 1);

    // Signboard
    for (int i = 0; i < signboard.materialCount; i++) {
        if (i >= 1 && i <= 5) {
            signboard.materials[i].maps[MATERIAL_MAP_DIFFUSE].color = player.levels[i - 1] == false ? RED : GREEN;
        }
    }
    DrawModel(signboard, (Vector3){ 0.0f, 0.0f, 9.0f}, 1.0f, WHITE);
}
