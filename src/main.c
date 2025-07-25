#include <stdio.h>
#include <float.h>
#include <stdbool.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <rcamera.h>

void CheckGround(BoundingBox);
void UpdateMouseCamera(Camera *);
void CheckCollision(BoundingBox, Camera *);
void LevelCollision(Model, Camera *);
void DrawLobby(Model);

// ===============================================================
// Starting player values
// ===============================================================

typedef enum Level {
    CURRENT,
    INCOMPLETE,
    COMPLETE
} Level;

typedef struct Player {
    Vector3 position;
    float speed;
    float sensitivity;
    float fovy;
    float size;
    float height;
    bool onGround;
    Level levels[5];
    float zSpeed;
    float zAcceleration;
} Player;

Player player = {
    .position = (Vector3){0.0f, 2.0f, 0.0f},
    .speed = 0.24f,
    .sensitivity = 10.0f,
    .fovy = 90.0f,
    .size = 0.5f,
    .height = 2.0f,
    .onGround = true,
    .levels = {CURRENT, INCOMPLETE, INCOMPLETE, INCOMPLETE, INCOMPLETE},
    .zSpeed = 0.0f,
};

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
    // Loading Up Models & Bounding Boxes
    // ===============================================================

    Model signboard = LoadModel("resources/Signboard.glb");
    signboard.transform = MatrixTranslate(0.0f, 0.0f, 9.0f);

    Model level1 = LoadModel("resources/Level1.glb");

    BoundingBox signboardBox = GetModelBoundingBox(signboard);
    BoundingBox platform = (BoundingBox){ (Vector3){ -10.0f, -2.0f, -10.0f }, (Vector3){ 10.0f, 0.0f, 10.0f } };

    // ===============================================================
    // Limit cursor to relative movement
    // ===============================================================
    
    DisableCursor();

    // ===============================================================
    // Main Loop
    // ===============================================================

    while (!WindowShouldClose()) {
        CheckGround(platform);
        CheckGround(signboardBox);
        // Update Camera
        UpdateMouseCamera(&camera);
        
        CheckCollision(signboardBox, &camera);
        CheckCollision(platform, &camera);
        LevelCollision(level1, &camera);

        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);

            DrawLobby(signboard);
            DrawModel(level1, (Vector3){ 0.0f, 0.0f, 0.0f }, 1, WHITE);

            DrawBoundingBox(signboardBox, GOLD);
            DrawBoundingBox(platform, GREEN);
            for (int i = 0; i < level1.meshCount; i++) {
                DrawBoundingBox(GetMeshBoundingBox(level1.meshes[i]), GOLD);
            }
        
            EndMode3D();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}

// ===============================================================
// Helper Functions
// ===============================================================

void CheckGround(BoundingBox box) {
    // Creating the player hitbox
    BoundingBox feetBox = (BoundingBox) {
        (Vector3){ player.position.x - player.size + 0.05f, player.position.y - 2.1f, player.position.z - player.size + 0.05f },
        (Vector3){ player.position.x + player.size - 0.05f, player.position.y - 2.0f, player.position.z + player.size - 0.05f}
    };
    player.onGround = CheckCollisionBoxes(feetBox, box) || player.onGround;
}

void UpdateMouseCamera(Camera *camera) {
    // ===============================================================
    // To make the camera follow the mouse
    // ===============================================================

    Vector2 delta = GetMouseDelta();
    delta.x *= player.sensitivity * DEG2RAD;
    delta.y *= player.sensitivity * DEG2RAD;

    // ===============================================================
    // Movement
    // ===============================================================

    Vector3 movement = {0};
    
    bool inStrafe = false;
    if (
        (IsKeyDown(KEY_W) || IsKeyDown(KEY_S))
        &&
        (IsKeyDown(KEY_D) || IsKeyDown(KEY_A))
    ) inStrafe = true;
    float strafeMultiplier = inStrafe ? 1 / sqrt(2.0f) : 1;    

    if (IsKeyDown(KEY_W)) {
        movement.x += player.speed * strafeMultiplier;
    }

    if (IsKeyDown(KEY_S)) {
        movement.x -= player.speed * strafeMultiplier;
    }

    if (IsKeyDown(KEY_D)) {
        movement.y += player.speed * strafeMultiplier;
    }

    if (IsKeyDown(KEY_A)) {
        movement.y -= player.speed * strafeMultiplier;
    }

    if (IsKeyPressed(KEY_SPACE) && player.onGround) {
        player.zSpeed = 0.25f; // Apply upward jump speed
        player.onGround = false; // Explicitly mark not grounded
    }

    // Apply gravity when not on ground
    if (!player.onGround) {
        player.zSpeed += -0.5f * GetFrameTime(); // Gravity acceleration
    }

    // Update movement
    movement.z = player.zSpeed;
    
    // ===============================================================
    // Camera Updation
    // ===============================================================
    
    UpdateCameraPro(camera, movement, (Vector3){delta.x, delta.y, 0.0f}, 0.0f);

    // Update player position
    player.position = camera->position;
}

void CheckCollision(BoundingBox box, Camera *camera) {
    // Creating the player hitbox
    BoundingBox playerBox = (BoundingBox) {
        (Vector3){ player.position.x - player.size, player.position.y - 2.0f, player.position.z - player.size },
        (Vector3){ player.position.x + player.size, player.position.y       , player.position.z + player.size}
    };

    if (CheckCollisionBoxes(box, playerBox)) {
        // Depth
        float px1 = box.max.x - playerBox.min.x; // left
        float px2 = playerBox.max.x - box.min.x; // right
        float py1 = box.max.y - playerBox.min.y; // below
        float py2 = playerBox.max.y - box.min.y; // above
        float pz1 = box.max.z - playerBox.min.z; // front
        float pz2 = playerBox.max.z - box.min.z; // back

        // Choose smallest the penetration axis
        float minPenetration = FLT_MAX;
        Vector3 resolution = { 0 };

        if (px1 >= 0 && px1 < minPenetration) { minPenetration = px1; resolution = (Vector3){ px1 , 0   , 0    }; }
        if (px2 >= 0 && px2 < minPenetration) { minPenetration = px2; resolution = (Vector3){ -px2, 0   , 0    }; }
        if (pz1 >= 0 && pz1 < minPenetration) { minPenetration = pz1; resolution = (Vector3){ 0   , 0   , pz1  }; }
        if (pz2 >= 0 && pz2 < minPenetration) { minPenetration = pz2; resolution = (Vector3){ 0   , 0   , -pz2 }; }
        if (py1 >= 0 && py1 < minPenetration) { minPenetration = py1; resolution = (Vector3){ 0   , py1 , 0    }; }
        if (py2 >= 0 && py2 < minPenetration) { minPenetration = py2; resolution = (Vector3){ 0   , -py2, 0    }; }

        // Move target too to avoid drift
        camera->target = Vector3Add(camera->target, resolution);
        camera->position = Vector3Add(camera->position, resolution);
    }

    // Sync position
    player.position = camera->position;
}

void LevelCollision(Model level, Camera *camera) {
    for (int i = 0; i < level.meshCount; i++) {
        BoundingBox box = GetMeshBoundingBox(level.meshes[i]);

        CheckCollision(box, camera);
        CheckGround(box);
    }
}

void DrawLobby(Model signboard) {
    // Platform
    DrawCube((Vector3){ 0.0f, -1.0f, 0.0f}, 20.0f, 2.0f, 20.0f, GRAY);
    DrawGrid(300, 1);

    // Signboard
    for (int i = 0; i < signboard.materialCount; i++) {
        if (i >= 1 && i <= 5) {
            signboard.materials[i].maps[MATERIAL_MAP_DIFFUSE].color =
                player.levels[i - 1] == CURRENT ? YELLOW : INCOMPLETE ? GRAY : GREEN;
        }
    }
    DrawModel(signboard, (Vector3){ 0 }, 1.0f, WHITE);
}
