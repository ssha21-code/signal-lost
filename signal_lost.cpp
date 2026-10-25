#include <raylib.h>
#include <raymath.h>
#include <memory>
#include <vector>
#include "ExtraHeader.h"
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

constexpr int WINDOW_WIDTH = 1000;
constexpr int WINDOW_HEIGHT = 800;
constexpr int DEFAULT_FPS = 60;

float MAP_W = 64;
int MAP_H = 64;
const int TILE_W = 50;
const int MAX_LIGHT_LEVEL = 10;
std::vector<int> map = std::vector<int>(MAP_W * MAP_H, 0);
std::vector<int> lightLevels = std::vector<int>(MAP_W * MAP_H, 0);
auto at = [](std::vector<int>& g, int x, int y) -> int& {
    if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) {
        std::abort();
    }
    return g[y * MAP_W + x];
};

const int FLOOR = 0;
const int SOLID = 1;

Texture2D texturePlayer;
Texture2D textureTile;
Texture2D textureSolid;
Texture2D textureTrap;
Texture2D textureHeart;
Texture2D textureOrb;
Texture2D textureBullet;
Texture2D textureGhost;
Texture2D textureArrow;
Texture2D textureTitleScreen;
Texture2D textureVictoryScreen;

Sound soundClick;
Sound soundFail;
Sound soundCollect;
Sound soundCollectHeart;
Sound soundShoot;
Sound soundHit;
Sound soundSnap;
Sound soundVictory;
Sound soundGlassBreaking;
Sound soundGhostWhisper;

Font font;

void loadAssets() {
    texturePlayer = LoadTexture("Textures\\player.png");
    textureTile = LoadTexture("Textures\\tile.png");
    textureSolid = LoadTexture("Textures\\solid.png");
    textureTrap = LoadTexture("Textures\\trap.png");
    textureHeart = LoadTexture("Textures\\heart.png");
    textureOrb = LoadTexture("Textures\\orb.png");
    textureBullet = LoadTexture("Textures\\bullet.png");
    textureGhost = LoadTexture("Textures\\ghost.png");
    textureArrow = LoadTexture("Textures\\arrow.png");
    textureTitleScreen = LoadTexture("Textures\\title_screen.png");
    textureVictoryScreen = LoadTexture("Textures\\victory_screen.png");

    soundClick = LoadSound("Sounds\\click.wav");
    soundFail = LoadSound("Sounds\\fail.wav");
    soundCollect = LoadSound("Sounds\\collect.wav");
    soundShoot = LoadSound("Sounds\\shoot.wav");
    soundHit = LoadSound("Sounds\\hit.wav");
    soundSnap = LoadSound("Sounds\\snap.wav");
    soundCollectHeart = LoadSound("Sounds\\collect_heart.wav");
    soundVictory = LoadSound("Sounds\\victory.wav");
    soundGlassBreaking = LoadSound("Sounds\\glass_breaking.wav");
    soundGhostWhisper = LoadSound("Sounds\\ghost_whisper.wav");

    font = LoadFontEx("Fonts\\font.ttf", 100, nullptr, 0);
}

class Player {
public: 
    Vector2 coords;
    Vector2 size;
    int hearts;
    int maxHearts = 3;
    int orbs = 0;
    float pointingAngle = 0.0f;

    bool isDead = false;
    bool shouldShoot = false;
    bool hasWon = false;
    
    Player(Vector2 coords) {
        this->coords = coords;
        this->size = {(float)texturePlayer.width, (float)texturePlayer.height};
        this->hearts = maxHearts;
    }

    void draw(float cameraOffsetX, float cameraOffsetY) {
        Rectangle src = {0, 0, size.x, size.y};
        Rectangle dest = {coords.x * TILE_W + (int)roundf(cameraOffsetX) + TILE_W / 25.0f, coords.y * TILE_W + (int)roundf(cameraOffsetY) + 2, TILE_W * 0.9f, TILE_W * 0.9f};
        DrawTexturePro(texturePlayer, src, dest, {0}, 0, WHITE);
    }
    void update(float cameraOffsetX, float cameraOffsetY, int orbsToPut) {
        if (IsKeyPressed(KEY_W) && at(map, (int)coords.x, (int)coords.y - 1) == FLOOR) {
            coords.y -= 1;
        }
        if (IsKeyPressed(KEY_S) && at(map, (int)coords.x, (int)coords.y + 1) == FLOOR) {
            coords.y += 1;
        }
        if (IsKeyPressed(KEY_A) && at(map, (int)coords.x - 1, (int)coords.y) == FLOOR) {
            coords.x -= 1;
        }
        if (IsKeyPressed(KEY_D) && at(map, (int)coords.x + 1, (int)coords.y) == FLOOR) {
            coords.x += 1;
        }

        Vector2 cameraOffset = {cameraOffsetX, cameraOffsetY};
        Vector2 worldPosition = Vector2Add(Vector2Scale(coords, TILE_W), {TILE_W * 0.5f, TILE_W * 0.5f});
        Vector2 mouseWorldPos = Vector2Subtract(GetMousePosition(), cameraOffset);
        Vector2 delta = Vector2Subtract(mouseWorldPos, worldPosition);
        pointingAngle = atan2(delta.y, delta.x) * RAD2DEG;

        if (IsKeyPressed(KEY_SPACE)) {
            shouldShoot = true;
            PlaySound(soundShoot);
        }

        if (hearts <= 0) {
            isDead = true;
        }

        if (orbs == orbsToPut) {
            hasWon = true;
            PlaySound(soundVictory);
        }
    }
    void drawDebugLines(float cameraOffsetX, float cameraOffsetY) {
        DrawRectangleLinesEx({coords.x * TILE_W + cameraOffsetX, coords.y * TILE_W + cameraOffsetY}, 2, RED);
        DrawCircleV({coords.x * TILE_W + cameraOffsetX, coords.y * TILE_W + cameraOffsetY}, 5, BLUE);
    }
};

struct Room {
    int x;
    int y;
    int w;
    int h;
};

class Arrow {
public: 
    Vector2 position;
    Vector2 size;
    Vector2 origin;
    float angleDeg;

    Arrow(Vector2 position, float angleDeg) {
        this->position = position;
        this->angleDeg = angleDeg;
        this->size = {(float)textureArrow.width, (float)textureArrow.height};
        this->origin = {size.x / 2.0f, size.y / 2.0f};
    }

    void draw() {
        Rectangle src = {0, 0, size.x, size.y};
        Rectangle dest = {position.x, position.y, size.x, size.y};
        DrawTexturePro(textureArrow, src, dest, origin, angleDeg, WHITE);
    }

    void update(Vector2 position, float angleDeg) {
        this->position = position;
        this->angleDeg = angleDeg;
    }
};

class Collectible {
    public: 
    Vector2 coords;

    bool shouldBeDestroyed = false;

    Collectible() {}

    virtual void draw(float cameraOffsetX, float cameraOffsetY, unsigned char alpha) = 0;
    void drawDebugLines(float cameraOffsetX, float cameraOffsetY) {
        DrawRectangleLinesEx({coords.x * TILE_W + cameraOffsetX, coords.y * TILE_W + cameraOffsetY, TILE_W, TILE_W}, 2.0f, RED);
        DrawCircleV({coords.x * TILE_W + cameraOffsetX, coords.y * TILE_W + cameraOffsetY}, 5, BLUE);
    }
};

class Trap : public Collectible {
public: 
    Trap (Vector2 coords) {
        this->coords = coords;
    }

    void draw(float cameraOffsetX, float cameraOffsetY, unsigned char alpha) {
        Rectangle src = {0, 0, (float)textureTrap.width, (float)textureTrap.height};
        Rectangle dest = {coords.x * TILE_W + cameraOffsetX, coords.y * TILE_W + cameraOffsetY, TILE_W, TILE_W};
        DrawTexturePro(textureTrap, src, dest, {0, 0}, 0, WHITE);
        DrawRectangleRec({coords.x * TILE_W + (int)cameraOffsetX, coords.y * TILE_W + (int)cameraOffsetY, TILE_W, TILE_W}, {0, 0, 0, alpha});
    }
};

class Heart : public Collectible {
public: 
    Heart (Vector2 coords) {
        this->coords = coords;
    }

    void draw(float cameraOffsetX, float cameraOffsetY, unsigned char lightLevel) {
        float light = lightLevel / (float)MAX_LIGHT_LEVEL;
        unsigned char v = (unsigned char)(255 * light);
        Rectangle src = {0, 0, (float)textureHeart.width, (float)textureHeart.height};
        Rectangle dest = {coords.x * TILE_W + (int)cameraOffsetX, coords.y * TILE_W + (int)cameraOffsetY, TILE_W, TILE_W};
        DrawTexturePro(textureHeart, src, dest, {0}, 0.0, {v, v, v, 255});
    }
};

class Orb : public Collectible {
public: 
    Arrow arrow = Arrow({0}, 0);

    Orb (Vector2 coords) {
        this->coords = coords;
    }

    void draw(float cameraOffsetX, float cameraOffsetY, unsigned char lightLevel) {
        Rectangle src = {0, 0, (float)textureOrb.width, (float)textureOrb.height};
        Rectangle dest = {coords.x * TILE_W + (int)cameraOffsetX, coords.y * TILE_W + (int)cameraOffsetY, TILE_W, TILE_W};
        DrawTexturePro(textureOrb, src, dest, {0}, 0.0, WHITE);
        arrow.draw();
    }
    void updateArrow(Vector2 playerCoords, float cameraOffsetX, float cameraOffsetY) {
        Vector2 worldGhostPosition = {
            coords.x * TILE_W + cameraOffsetX + TILE_W / 2.0f,
            coords.y * TILE_W + cameraOffsetY + TILE_W / 2.0f
        };
        Vector2 worldPlayerPosition = {
            playerCoords.x * TILE_W + cameraOffsetX + TILE_W / 2.0f,
            playerCoords.y * TILE_W + cameraOffsetY + TILE_W / 2.0f
        };
        Vector2 delta = Vector2Subtract(worldGhostPosition, worldPlayerPosition);
        float arrowAngleRad = atan2(delta.y, delta.x);
        float playerArrowOffset = 300.0f;
        Vector2 arrowPosition = {
            worldPlayerPosition.x + cosf(arrowAngleRad) * playerArrowOffset,
            worldPlayerPosition.y + sinf(arrowAngleRad) * playerArrowOffset,
        };
        float arrowAngleDeg = arrowAngleRad * RAD2DEG;
        arrow.update(arrowPosition, arrowAngleDeg);
    }
};

class Bullet {
public: 
    Vector2 position;
    Vector2 size;
    Vector2 origin;
    Vector2 subtraction;
    std::array<Vector2, 4> rectCorners;
    float angleDeg;
    static constexpr float VELOCITY = 10.0f;

    bool shouldBeDestroyed = false;
    bool isPlayerBullet;

    Bullet(Vector2 center, float angleDeg, bool isPlayerBullet) {
        this->size = {(float)textureBullet.width, (float)textureBullet.height};
        this->subtraction = {
            cosf(angleDeg * DEG2RAD) * size.x / 2.0f,
            sinf(angleDeg * DEG2RAD) * size.x / 2.0f
        };
        this->position = Vector2Subtract(center, subtraction);
        this->origin = Vector2Scale(size, 1.0f / 2.0f);
        this->angleDeg = angleDeg;
        this->isPlayerBullet = isPlayerBullet;
    }

    void draw(float cameraOffsetX, float cameraOffsetY) {
        Rectangle src = {0, 0, size.x, size.y};
        Rectangle dest = {position.x + cameraOffsetX, position.y + cameraOffsetY, size.x * TILE_W / 50.0f, size.y * TILE_W / 50.0f};
        DrawTexturePro(textureBullet, src, dest, origin, angleDeg, isPlayerBullet ? GREEN : WHITE);
    }
    void update(float cameraOffsetX, float cameraOffsetY) {
        Vector2 velocity = {
            cos(angleDeg * DEG2RAD) * VELOCITY * DEFAULT_FPS * GetFrameTime(),
            sin(angleDeg * DEG2RAD) * VELOCITY * DEFAULT_FPS * GetFrameTime()
        };
        position = Vector2Add(position, velocity);

        std::array<Vector2, 4> localCorners = {
            Vector2{ -origin.x, -origin.y },
            Vector2{  origin.x, -origin.y },
            Vector2{  origin.x,  origin.y },
            Vector2{ -origin.x,  origin.y }
        };
        for (int i = 0; i < 4; i++) {
            Vector2 rotated = Vector2Rotate(localCorners[i], angleDeg * DEG2RAD);
            rectCorners[i] = Vector2Add(Vector2Add(position, {cameraOffsetX, cameraOffsetY}), rotated);
        }
    }
    void drawDebugLines(float cameraOffsetX, float cameraOffsetY) {
        for (int i = 0; i < rectCorners.size(); i++) {
            DrawLineEx(rectCorners.at(i), rectCorners.at(i != 3 ? i+1 : 0), 2, RED);
        }
        DrawCircleV(position, 5, BLUE);
    }
};

class Ghost {
public: 
    Vector2 coords;
    float lastShootTime = 0.0f;
    float shootCooldown;
    float lookingAngle = 0.0f;
    Room room;
    
    bool shouldShoot = false;
    bool shouldBeDestroyed = false;

    Ghost(Vector2 coords, Room room) {
        this->coords = coords;
        this->shootCooldown = GetRandomValue(15, 30) / 10.0f;
        this->room = room;
    }

    void draw(float cameraOffsetX, float cameraOffsetY, unsigned char lightLevel) {
        float light = lightLevel / (float)MAX_LIGHT_LEVEL;
        unsigned char v = (unsigned char)(255 * light);
        Rectangle src = {0, 0, (float)textureGhost.width, (float)textureGhost.height};
        Rectangle dest = {coords.x * TILE_W + cameraOffsetX, coords.y * TILE_W + cameraOffsetY, TILE_W, TILE_W};
        DrawTexturePro(textureGhost, src, dest, {0, 0}, 0, {v, v, v, 255});
    }
    void update(float timeElapsed, Vector2 playerCoords, float cameraOffsetX, float cameraOffsetY) {
        Vector2 worldGhostPosition = {
            coords.x * TILE_W + cameraOffsetX + TILE_W / 2.0f,
            coords.y * TILE_W + cameraOffsetY + TILE_W / 2.0f
        };
        Vector2 worldPlayerPosition = {
            playerCoords.x * TILE_W + cameraOffsetX + TILE_W / 2.0f,
            playerCoords.y * TILE_W + cameraOffsetY + TILE_W / 2.0f
        };
        Vector2 delta = Vector2Subtract(worldPlayerPosition, worldGhostPosition);
        lookingAngle = atan2(delta.y, delta.x) * RAD2DEG;

        if (timeElapsed - lastShootTime > shootCooldown && pointIsInRoom(playerCoords, room)) {
            shouldShoot = true;
            lastShootTime = timeElapsed;
        }
    }

    bool pointIsInRoom(Vector2 point, Room room) {
        return !(point.x + 1 < room.x || point.x - 1 >= room.x + room.w ||
            point.y + 1 < room.y || point.y - 1 >= room.y + room.h
        );
    }
    void drawDebugLines(float cameraOffsetX, float cameraOffsetY) {
        DrawRectangleLinesEx({coords.x * TILE_W + cameraOffsetX, coords.y * TILE_W + cameraOffsetY}, 2, RED);
        DrawCircleV({coords.x * TILE_W + cameraOffsetX, coords.y * TILE_W + cameraOffsetY}, 5, BLUE);
    }
};

class Game {
public:
    Player player = Player({0});
    float cameraOffsetX = 0.0f;
    const float cameraFollowX = WINDOW_WIDTH / 2.0f;
    float cameraOffsetY = 0.0f;
    const float cameraFollowY = WINDOW_HEIGHT / 2.0f;
    float timeElapsed;
    float startTime;
    float endTime;
    float timeLastPlayedSound;
    float soundInterval;
    std::vector<Room> rooms;
    std::vector<Trap> traps;
    std::vector<Heart> hearts;
    std::vector<Orb> orbs;
    std::vector<Bullet> bullets;
    std::vector<Ghost> ghosts;
    int orbsPut = 0;
    int orbsToPut = 5;
    int roomsPut = 0;
    float roomsToPut = 10;
    float map_w = 64;
    float map_h = 64;
    float playerHearts = 3;
    int canPlayScarySounds = 1;

    bool isDebugging = false;
    bool isInTitleScreen = true;

    Rectangle playButtonBounds = {90.0f, 608.0f, 286.0f, 130.0f};
    Rectangle sliderBarBounds = {650.0f, 440.0f, 250.0f, 35.0f};

    Game() {

    }

    void draw() {
        if (isInTitleScreen) {
            setGuiTheme();
            DrawTexture(textureTitleScreen, 0, 0, WHITE);
            GuiSliderBar(sliderBarBounds, "", "", &roomsToPut, 5.0f, 30.0f);
            roomsToPut = roundf(roomsToPut);
            DrawTextEx(font, TextFormat("Rooms: %i", (int)roomsToPut), {sliderBarBounds.x + 10, sliderBarBounds.y + 5}, 25.0f, 1.0f, WHITE);
            
            GuiSliderBar(changePosRectangle(sliderBarBounds, {sliderBarBounds.x, sliderBarBounds.y + 70.0f}), "", "", &map_w, 48.0f, 128.0f);
            map_w = (int)map_w;
            map_h = map_w;
            DrawTextEx(font, TextFormat("Map Size: %i", (int)map_w), {sliderBarBounds.x + 10, sliderBarBounds.y + 75}, 25.0f, 1.0f, WHITE);
            
            float orbsToPutVar = orbsToPut;
            GuiSliderBar(changePosRectangle(sliderBarBounds, {sliderBarBounds.x, sliderBarBounds.y + 140.0f}), "", "", &orbsToPutVar, 1.0f, 20.0f);
            orbsToPut = orbsToPutVar;
            DrawTextEx(font, TextFormat("Orbs: %i", (int)orbsToPut), {sliderBarBounds.x + 10, sliderBarBounds.y + 145}, 25.0f, 1.0f, WHITE);
            
            GuiSliderBar(changePosRectangle(sliderBarBounds, {sliderBarBounds.x, sliderBarBounds.y + 210.0f}), "", "", &playerHearts, 1.0f, 10.0f);
            DrawTextEx(font, TextFormat("Hearts: %i", (int)playerHearts), {sliderBarBounds.x + 10, sliderBarBounds.y + 215.0f}, 25.0f, 1.0f, WHITE);
            playerHearts = (int)playerHearts;

            DrawTextEx(font, "Play Sound Effects", {sliderBarBounds.x, sliderBarBounds.y + 280.0f}, 25.0f, 1.0f, WHITE);
            GuiToggleSlider({sliderBarBounds.x + 260, sliderBarBounds.y + 280.0f, 60, 20}, "OFF;ON", &canPlayScarySounds);
        } else if (player.hasWon) {
            DrawTexture(textureVictoryScreen, 0, 0, WHITE);
            int timeTaken = endTime - startTime;
            const char* timeString = TextFormat("%i min %i sec", timeTaken / 60, timeTaken % 60);
            const char* text = TextFormat("Time Taken: %s", timeString);
            DrawTextEx(font, text, {450.0f, 680.0f}, 45.0f, 1.0f, WHITE);
            DrawTextEx(font, "Made by @ssha21-code", {50.0f, 750.0f}, 25.0f, 1.0f, WHITE);
        } else {
            for (int x = 0; x < MAP_W; x++) {
                for (int y = 0; y < MAP_H; y++) {
                    unsigned char alpha = 255.0f - (at(lightLevels, x, y) * 25.5f);
                    Color color;
                    float camX = (int)roundf(cameraOffsetX);
                    float camY = (int)roundf(cameraOffsetY);
                    switch (at(map, x, y)) {
                        case FLOOR: {
                            Rectangle src = {0, 0, (float)textureTile.width, (float)textureTile.height};
                            Rectangle dest = {x * TILE_W + camX, y * TILE_W + camY, TILE_W, TILE_W};
                            DrawTexturePro(textureTile, src, dest, {0, 0}, 0, WHITE);
                            break;
                        }
                        case SOLID: {
                            Rectangle src = {0, 0, (float)textureSolid.width, (float)textureSolid.height};
                            Rectangle dest = {x * TILE_W + camX, y * TILE_W + camY, TILE_W, TILE_W};
                            DrawTexturePro(textureSolid, src, dest, {0, 0}, 0, WHITE);
                            break;
                        }
                    }
                    DrawRectangleRec({(float)x * TILE_W + camX, (float)y * TILE_W + camY, TILE_W, TILE_W}, {0, 0, 0, alpha});
                }
                
            }
            int camX = (int)roundf(cameraOffsetX);
            int camY = (int)roundf(cameraOffsetY);
            for (auto& trap: traps) {
                unsigned char alpha = 255.0f - (at(lightLevels, (int)trap.coords.x, (int)trap.coords.y) * 25.5f);
                trap.draw(camX, camY, alpha);
                if (isDebugging) trap.drawDebugLines(camX, camY);
            }
            for (auto& heart: hearts) {
                heart.draw(camX, camY, at(lightLevels, (int)heart.coords.x, (int)heart.coords.y));
                if (isDebugging) heart.drawDebugLines(camX, camY);
            }
            for (auto& orb: orbs) {
                orb.updateArrow(player.coords, camX, camY);
                orb.draw(camX, camY, 0);
                if (isDebugging) orb.drawDebugLines(camX, camY);
            }
            for (auto& bullet: bullets) {
                bullet.draw(camX, camY);
                if (isDebugging) bullet.drawDebugLines(camX, camY);
            }
            for (auto& ghost: ghosts) {
                ghost.draw(camX, camY, at(lightLevels, (int)ghost.coords.x, (int)ghost.coords.y));
                if (isDebugging) ghost.drawDebugLines(camX, camY);
            }
            player.draw(camX, camY);
            if (isDebugging) player.drawDebugLines(camX, camY);
            int heartWidth = textureHeart.width;
            for (int i = 0; i < player.hearts; i++) {
                DrawTexture(textureHeart, 70 + (heartWidth + 20) * i, 50, WHITE);
            }
            DrawTextEx(font, TextFormat("%d/%d", player.orbs, orbsToPut), {170, 140}, 50.0f, 1.0f, WHITE);
            Rectangle src = {0, 0, (float)textureOrb.width, (float)textureOrb.height};
            Rectangle dest = {100.0f, 135.0f, 50.0f, 50.0f};
            DrawTexturePro(textureOrb, src, dest, {0}, 0.0, WHITE);
    
            Vector2 playerPosition = Vector2Add(Vector2Scale(player.coords, TILE_W), {cameraOffsetX, cameraOffsetY});
            Vector2 playerCenter = Vector2Add(playerPosition, {TILE_W / 2.0f, TILE_W / 2.0f});
        }
    }
    void update() {
        Vector2 mousePos = GetMousePosition();

        if (isInTitleScreen) {
            if (CheckCollisionPointRec(mousePos, playButtonBounds) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                isInTitleScreen = !isInTitleScreen;
                startTime = timeElapsed;
                PlaySound(soundClick);
                reset();
            }
        } else if (player.hasWon) {
            
        } else {
            timeElapsed += GetFrameTime();
    
            player.update(cameraOffsetX, cameraOffsetY, orbsToPut);
    
            if (IsKeyPressed(KEY_TAB)) {
                isDebugging = !isDebugging;
            }

            if (GetTime() - timeLastPlayedSound >= soundInterval && canPlayScarySounds) {
                if (GetRandomValue(0, 1)) {
                    PlaySound(soundGlassBreaking);
                } else {
                    PlaySound(soundGhostWhisper);
                }
                timeLastPlayedSound = GetTime();
            }
    
            for (int x = 0; x < MAP_W; x++) {
                for (int y = 0; y < MAP_H; y++) {
                    at(lightLevels, x, y) = 1;
                }
            }
    
            correctCameraOffset(false);
    
            if (player.shouldShoot) {
                Vector2 playerPosition = Vector2Scale(player.coords, TILE_W);
                Vector2 playerCenter = Vector2Add(playerPosition, {TILE_W / 2.0f, TILE_W / 2.0f});
                bullets.push_back(Bullet(playerCenter, player.pointingAngle, true));
                player.shouldShoot = false;
            }
    
            for (auto& orb: orbs) {
                lightUpPoint(orb.coords, 1);
            }
            for (auto& bullet: bullets) {
                bullet.update(cameraOffsetX, cameraOffsetY);
            }
            for (auto& ghost: ghosts) {
                ghost.update(timeElapsed, player.coords, cameraOffsetX, cameraOffsetY);
                if (ghost.shouldShoot) {
                    Vector2 ghostPosition = Vector2Scale(ghost.coords, TILE_W);
                    Vector2 ghostCenter = Vector2Add(ghostPosition, {TILE_W / 2.0f, TILE_W / 2.0f});
                    bullets.push_back(Bullet(ghostCenter, ghost.lookingAngle, false));
                    PlaySound(soundShoot);
                    ghost.shouldShoot = false;
                }
            }

            if (player.isDead) {
                reset();
                PlaySound(soundFail);
                isInTitleScreen = true;
            }

            if (player.hasWon) {
                endTime = timeElapsed;
            }
    
            lightUpPoint(player.coords, 2);
    
            checkForRemoval();
            checkForCollisions();
            garbageCollect();
        }
    }
    void correctCameraOffset(bool instant) {
        float targetCameraOffsetX = 0.0f;
        if (player.coords.x * TILE_W > cameraFollowX) {
            targetCameraOffsetX = cameraFollowX - player.coords.x * TILE_W;
        }
        float targetCameraOffsetY = 0.0f;
        if (player.coords.y * TILE_W > cameraFollowY) {
            targetCameraOffsetY = cameraFollowY - player.coords.y * TILE_W;
        }

        cameraOffsetX += (targetCameraOffsetX - cameraOffsetX) * (instant ? 1.0f : 0.1f) * DEFAULT_FPS * GetFrameTime();
        cameraOffsetY += (targetCameraOffsetY - cameraOffsetY) * (instant ? 1.0f : 0.1f) * DEFAULT_FPS * GetFrameTime();
    }
    void lightUpPoint(Vector2 point, int strength) {
        int px = (int)point.x;
        int py = (int)point.y;

        const int radius = strength;
        for (int dx = -radius; dx <= radius; dx++) {
            for (int dy = -radius; dy <= radius; dy++) {
                int x = px + dx;
                int y = py + dy;
                if (x < 0 || x >= MAP_W || y < 0 || y >= MAP_H) continue;

                int dist = abs(dx) + abs(dy);
                // int dist = sqrtf((float)(dx * dx + dy * dy));
                int light = (int)(6 - (int)(dist / 2) * 2);
                if (light <= 2) continue;

                at(lightLevels, x, y) = ClampInt(
                    at(lightLevels, x, y) + light,
                    0, 
                    MAX_LIGHT_LEVEL
                );
            }
        }
    }
    int ClampInt(int value, int min, int max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
    Room makeRandomRoom() {
        Room r;
        r.w = GetRandomValue(4, 10);
        r.h = GetRandomValue(4, 10);
        r.x = GetRandomValue(1, MAP_W - r.w - 1);
        r.y = GetRandomValue(1, MAP_H - r.h - 1);
        return r;
    }
    bool overlaps(const Room& a, const Room& b) {
        const int padding = 1;

        return !(
            a.x + a.w + padding <= b.x ||
            b.x + b.w + padding <= a.x ||
            a.y + a.h + padding <= b.y ||
            b.y + b.h + padding <= a.y
        );
    }
    bool checkEligibleForHeart(const Room& r) {
        if (r.x + r.w < 15 || r.x + r.w > MAP_W - 10 || r.y + r.h < 15 || r.y + r.h > MAP_W - 10) {
            return true;
        }
        return false;
    }
    void carveRoom(const Room& r) {
        for (int x = r.x; x < r.x + r.w; x++) {
            for (int y = r.y; y < r.y + r.h; y++) {
                at(map, x, y) = 0;
            }
        }
    }
    void carveCorridor(Vector2 a, Vector2 b) {
        int x1 = (int)a.x;
        int x2 = (int)b.x;
        int y1 = (int)a.y;
        int y2 = (int)b.y;

        for (int x = std::min(x1, x2); x <= std::max(x1, x2); x++) {
            at(map, x, y1) = 0;
        }
        for (int y = std::min(y1, y2); y <= std::max(y1, y2); y++) {
            at(map, x2, y) = 0;
        }
    }
    Vector2 centerRoom(const Room& r) {
        return {
            (float)(r.x + r.w / 2),
            (float)(r.y + r.h / 2)
        };
    }
    void addTraps(Room room) {
        int numTraps = GetRandomValue(5, 10);
        std::vector<Vector2> usedCoords;
        for (int i = 0; i < numTraps; i++) {
            bool ok = true;
            Vector2 coords = {
                (float)GetRandomValue(room.x + 1, room.x + room.w - 2),
                (float)GetRandomValue(room.y + 1, room.y + room.h - 2)
            };
            for (auto& oldCoords: usedCoords) {
                if (oldCoords == coords) {
                    ok = false;
                    break;
                }
            }
            if (ok) {
                traps.push_back(Trap(coords));
                usedCoords.push_back(coords);
            }
        }
    }
    void addGhost(Room room) {
        Vector2 spawnCoords = {
            (float)GetRandomValue(room.x + 1, room.x + room.w - 2),
            (float)GetRandomValue(room.y + 1, room.y + room.h - 2)
        };
        ghosts.push_back(Ghost(spawnCoords, room));
    }
    void addHeart(Room room) {
        Vector2 coords = {
            (float)GetRandomValue(room.x, room.x + room.w - 1),
            (float)GetRandomValue(room.y, room.y + room.h - 1)
        };
        hearts.push_back(Heart(coords));
    }
    void addOrb(Room room) {
        Vector2 coords = {
            (float)GetRandomValue(room.x, room.x + room.w - 1),
            (float)GetRandomValue(room.y, room.y + room.h - 1)
        };
        orbs.push_back(Orb(coords));
    }
    Rectangle changePosRectangle(Rectangle rectangle, Vector2 position) {
        return Rectangle{position.x, position.y, rectangle.width, rectangle.height};
    }
    void checkForRemoval() {
        for (auto& bullet: bullets) {
            Vector2 bulletCoords = Vector2Scale(bullet.position, 1.0f / TILE_W);
            bulletCoords = {
                floorf(bulletCoords.x),
                floorf(bulletCoords.y)
            };
            if (at(map, (int)bulletCoords.x, (int)bulletCoords.y) == SOLID) {
                bullet.shouldBeDestroyed = true;
            }
        }
    }
    void checkForCollisions() {
        for (auto& trap: traps) {
            if (player.coords == trap.coords) {
                trap.shouldBeDestroyed = true;
                player.hearts -= 1;
                PlaySound(soundSnap);
            }
        }
        for (auto& heart: hearts) {
            if (player.coords == heart.coords) {
                heart.shouldBeDestroyed = true;
                player.hearts = Clamp(player.hearts + 1, 0, playerHearts);
                PlaySound(soundCollectHeart);
            }
        }
        for (auto& orb: orbs) {
            if (player.coords == orb.coords) {
                orb.shouldBeDestroyed = true;
                player.orbs = Clamp(player.orbs + 1, 0, orbsToPut);
                PlaySound(soundCollect);
            }
        }
        for (auto& bullet: bullets) {
            if (Collision::CheckCollisionRectCornersRec({player.coords.x * TILE_W + cameraOffsetX, player.coords.y * TILE_W + cameraOffsetY, TILE_W, TILE_W}, bullet.rectCorners) && !bullet.isPlayerBullet) {
                player.hearts -= 1;
                bullet.shouldBeDestroyed = true;
                PlaySound(soundHit);
            }
            Vector2 bulletCoords = Vector2Scale(bullet.position, 1.0f / TILE_W);
            int bx = (int)floorf(bulletCoords.x);
            int by = (int)floorf(bulletCoords.y);
            if (bx < 0 || bx >= MAP_W || by < 0 || by >= MAP_H) {
                bullet.shouldBeDestroyed = true;
                continue;
            }
        }
        for (auto& ghost: ghosts) {
            for (auto& bullet: bullets) {
                if (Collision::CheckCollisionRectCornersRec({ghost.coords.x * TILE_W + cameraOffsetX, ghost.coords.y * TILE_W + cameraOffsetY, TILE_W, TILE_W}, bullet.rectCorners) && bullet.isPlayerBullet) {
                    ghost.shouldBeDestroyed = true;
                    bullet.shouldBeDestroyed = true;
                    PlaySound(soundHit);
                }
            }
        }
    }
    void garbageCollect() {
        for (int i = 0; i < traps.size(); i++) {
            if (traps.at(i).shouldBeDestroyed) {
                traps.erase(traps.begin() + i);
                i--;
            }
        }
        for (int i = 0; i < hearts.size(); i++) {
            if (hearts.at(i).shouldBeDestroyed) {
                hearts.erase(hearts.begin() + i);
                i--;
            }
        }
        for (int i = 0; i < orbs.size(); i++) {
            if (orbs.at(i).shouldBeDestroyed) {
                orbs.erase(orbs.begin() + i);
                i--;
            }
        }
        for (int i = 0; i < bullets.size(); i++) {
            if (bullets.at(i).shouldBeDestroyed) {
                bullets.erase(bullets.begin() + i);
                i--;
            }
        }
        for (int i = 0; i < ghosts.size(); i++) {
            if (ghosts.at(i).shouldBeDestroyed) {
                ghosts.erase(ghosts.begin() + i);
                i--;
            }
        }
    }
    void reset() {
        rooms.clear();
        traps.clear();
        hearts.clear();
        orbs.clear();
        bullets.clear();
        ghosts.clear();
        orbsPut = 0;
        roomsPut = 0;
        MAP_W = map_w;
        MAP_H = map_h;
        timeLastPlayedSound = GetTime();
        soundInterval = GetRandomValue(10, 20);
        map = std::vector<int>(MAP_W * MAP_H, 0);
        lightLevels = std::vector<int>(MAP_W * MAP_H, 0);
        for (int x = 0; x < MAP_W; x++) {
            for (int y = 0; y < MAP_H; y++) {
                at(map, x, y) = 1;
            }
        }
        for (int x = 0; x < MAP_W; x++) {
            for (int y = 0; y < MAP_H; y++) {
                at(lightLevels, x, y) = 0;
            }
        }
        rooms.push_back(Room{(int)(MAP_W - 10) / 2, (int)(MAP_W - 10) / 2, 10, 10});
        carveRoom(rooms.at(0));
        while (roomsPut < roomsToPut - 1) {
            Room r = makeRandomRoom();
            bool ok = true;
            for (auto& other: rooms) {
                if (overlaps(r, other)) {
                    ok = false;
                    break;
                }
            }

            if (ok) {
                carveRoom(r);
                rooms.push_back(r);
                addTraps(r);
                roomsPut++;
                if (GetRandomValue(1, 3) < 3) {
                    addGhost(r);
                }
                if (checkEligibleForHeart(r)) {
                    addHeart(r);
                    addOrb(r);
                    orbsPut++;
                }
            }
        }
        if (orbsPut < orbsToPut) {
            int orbsAdded = 0;
            for (int i = 0; i < orbsToPut - orbsPut; i++) {
                Room room;
                if ((rooms.size() - 1 - i) < 1 || (rooms.size() - 1 - i) > rooms.size() - 1) {
                    room = rooms.at(GetRandomValue(0, rooms.size() - 1));
                } else {
                    room = rooms.at(rooms.size() - 1 - i);
                }
                Vector2 coords = {
                    (float)GetRandomValue(room.x, room.x + room.w - 1),
                    (float)GetRandomValue(room.y, room.y + room.h - 1)
                };
                orbs.push_back(Orb(coords));
                orbsAdded++;
            }
            orbsPut += orbsAdded;
        }
        if (orbs.size() > orbsToPut) {
            orbs.erase(orbs.begin() + orbsToPut, orbs.end());
        }
        for (auto& trap: traps) {
            for (auto &heart: hearts) {
                for (auto &orb: orbs) {
                    if (trap.coords == heart.coords) {
                        trap.shouldBeDestroyed = true;
                    }
                    if (trap.coords == orb.coords) {
                        trap.shouldBeDestroyed = true;
                    }
                    if (heart.coords == orb.coords) {
                        heart.shouldBeDestroyed = true;
                    }
                }
            }
        }
        for (size_t i = 1; i < rooms.size(); i++) {
            carveCorridor(centerRoom(rooms.at(i - 1)), centerRoom(rooms.at(i)));
        }

        player = Player(centerRoom(rooms.at(0)));
        player.hearts = playerHearts;
        correctCameraOffset(true);
    }
    void setGuiTheme() {
        GuiSetStyle(SLIDER, BORDER_WIDTH, 2);
        GuiSetStyle(SLIDER, BORDER_COLOR_NORMAL, 0x4e010dff);
        GuiSetStyle(SLIDER, BORDER_COLOR_FOCUSED, 0x4e010dff);
        GuiSetStyle(SLIDER, BORDER_COLOR_PRESSED, 0x4e010dff);
        GuiSetStyle(SLIDER, BORDER_COLOR_DISABLED, 0x4e010dff);

        GuiSetStyle(SLIDER, BASE_COLOR_NORMAL, 0xFF7F27ff);
        GuiSetStyle(SLIDER, BASE_COLOR_FOCUSED, 0x4e010dff);
        GuiSetStyle(SLIDER, BASE_COLOR_PRESSED, 0x4e010dff);
        GuiSetStyle(SLIDER, BASE_COLOR_DISABLED, 0xFF7F27ff);

        GuiSetStyle(SLIDER, TEXT_COLOR_NORMAL, 0x4e010dff);
        GuiSetStyle(SLIDER, TEXT_COLOR_FOCUSED, 0x4e010dff);
        GuiSetStyle(SLIDER, TEXT_COLOR_PRESSED, 0x4e010dff);
        GuiSetStyle(SLIDER, TEXT_COLOR_DISABLED, 0x4e010dff);

        GuiSetStyle(TOGGLE, BASE_COLOR_NORMAL, 0xFF7F27ff);
        GuiSetStyle(TOGGLE, BASE_COLOR_FOCUSED, 0xFF7F27ff);
        GuiSetStyle(TOGGLE, BASE_COLOR_PRESSED, 0xFF7F27ff);
        GuiSetStyle(TOGGLE, BASE_COLOR_DISABLED, 0xFF7F27ff);

        GuiSetStyle(TOGGLE, BORDER_COLOR_NORMAL, 0xFF7F27ff);
        GuiSetStyle(TOGGLE, BORDER_COLOR_FOCUSED, 0xFF7F27ff);
        GuiSetStyle(TOGGLE, BORDER_COLOR_PRESSED, 0xFF7F27ff);
        GuiSetStyle(TOGGLE, BORDER_COLOR_DISABLED, 0xFF7F27ff);
    }
};

int main() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Signal Lost");
    InitAudioDevice();
    SetTargetFPS(DEFAULT_FPS);

    loadAssets();
    Game game = Game();

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        
        game.update();
        game.draw();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}