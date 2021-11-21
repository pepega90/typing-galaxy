#include "raylib.h"
#include "math.h"
#include "raymath.h"
#include <iostream>
#include <string>
#include <fstream>

#define METEOR_ALIVE 3
using namespace std;

// setup window
const int WIDTH{ 600 };
const int HEIGHT{ 480 };

struct Bullet
{
    Texture2D img;
    Rectangle rect;
    int speedX{ 0 };
    int speedY{ 0 };
    bool active;

    void shoot()
    {
        rect.x += speedX;
        rect.y -= speedY;
    }

    void resetPosition(Vector2 gunPos)
    {
        active = true;
        rect.x = gunPos.x;
        rect.y = gunPos.y + 3;
    }
};


struct Meteor
{
    Texture2D img;
    Rectangle rect;
    bool alive;
    string kata;
    int live;
    Vector2 lastPos{};
    Rectangle meteorSource;
    bool explode;
};

int main()
{
    InitWindow(WIDTH, HEIGHT, "Typing galaxy");
    InitAudioDevice();
    SetTargetFPS(60);

    Texture2D background = LoadTexture("texture/bg.png");
    // canon, base untuk gun
    Texture2D canon = LoadTexture("texture/ufoYellow.png");
    Vector2 canonPos = { 20, HEIGHT / 2 };
    Rectangle canonRect = { canonPos.x, canonPos.y, (float)canon.width * .5f, (float)canon.height * .5f };

    // rotate gambar 90deg counter clockwise
    Image gunRotate = LoadImage("texture/gun00.png");
    ImageRotateCCW(&gunRotate);
    Texture2D gun = LoadTextureFromImage(gunRotate);
    Rectangle sourceRec = { 0.0f, 0.0f, (float)gun.width, (float)gun.height };
    Rectangle gunPos = { 40, HEIGHT / 2 + 23, gun.width * .9f, gun.height * .9f };

    // laser untuk peluru 
    Image laserRotate = LoadImage("texture/laser.png");
    ImageRotateCCW(&laserRotate);
    Bullet laser;
    laser.img = LoadTextureFromImage(laserRotate);
    laser.rect.x = gunPos.x;
    laser.rect.y = gunPos.y + 3;
    laser.rect.width = laser.img.width;
    laser.rect.height = laser.img.height;
    Rectangle laserSource = { 0.0f, 0.0f, (float)laser.img.width, (float)laser.img.height };


    // initiate meteorList
    Meteor meteorList[METEOR_ALIVE];
    // reading txt file, untuk mendapatkan list kata
    int x = 0;
    string arr[100];

    string kalimat;
    ifstream baca("wordList.txt");
    if (baca.is_open())
    {
        while (getline(baca, kalimat))
        {
            arr[x] = kalimat;
            x++;
        }
    }

    // Meteor explosion
    int regularExpFrame{ 9 };
    int regularCurrentFrame{ 0 };
    float regularLastUpdate{ 0.02 };
    float regularRunningTime{};

    Texture2D regularExp[regularExpFrame]{
        LoadTexture("texture/regularExplosion00.png"),
        LoadTexture("texture/regularExplosion01.png"),
        LoadTexture("texture/regularExplosion02.png"),
        LoadTexture("texture/regularExplosion03.png"),
        LoadTexture("texture/regularExplosion04.png"),
        LoadTexture("texture/regularExplosion05.png"),
        LoadTexture("texture/regularExplosion06.png"),
        LoadTexture("texture/regularExplosion07.png"),
        LoadTexture("texture/regularExplosion08.png"),
    };

    Rectangle regularExpRect{ 0, 0, (float)regularExp[1].width, (float)regularExp[1].height };

    // LoadSound
    Sound explosionSfx = LoadSound("sfx/Explosion2.wav");
    Sound typingSFX = LoadSound("sfx/typingSFX.mp3");

    // Game variable
    float deltaX, deltaY, angle, deg = 0;
    int valChar;
    float rotation = 0;
    bool cek{};
    int idx;
    int wpm{};
    bool gameOver{};
    bool startGame{};

    // membuat countdown
    int countdown = 20;
    float lastUpdateCountdown{ 1.0 };
    float runCountDown{};

    // inisialisasi array meteorList
    for (int i = 0; i < METEOR_ALIVE; i++)
    {
        meteorList[i].img = LoadTexture("texture/meteorBrown_med1.png");
        meteorList[i].rect.x = 610;
        meteorList[i].rect.y = GetRandomValue(10, 450);
        meteorList[i].rect.width = meteorList[i].img.width;
        meteorList[i].rect.height = meteorList[i].img.height;
        meteorList[i].alive = true;
        meteorList[i].explode = false;
        meteorList[i].kata = arr[GetRandomValue(0, x - 1)];
        meteorList[i].live = meteorList[i].kata.length();
        meteorList[i].meteorSource = { 0.0f, 0.0f, (float)meteorList[i].img.width, (float)meteorList[i].img.height };
    }

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        ClearBackground(BLACK);
        BeginDrawing();
        // draw background
        DrawTextureEx(background, Vector2{}, 0.0, 1.0, WHITE);

        valChar = GetCharPressed();

        // restart game
        if (IsKeyPressed(KEY_R) && gameOver)
        {
            for (int i = 0; i < METEOR_ALIVE; i++)
            {
                meteorList[i].alive = false;
            }

            gameOver = false;
            countdown = 20;
            wpm = 0;
        }
        // start game
        if (IsKeyPressed(KEY_SPACE) && !startGame)
        {
            startGame = true;
            laser.active = true;
        }


        // main menu scene
        if (!startGame)
        {

            DrawText("Typing Galaxy", WIDTH / 2 - 135, HEIGHT / 4, 40, YELLOW);
            DrawText("Press \"SPACE\" untuk start game", WIDTH / 2 - 200, HEIGHT / 2, 25, WHITE);
            DrawText("Created by aji mustofa @pepega90", WIDTH / 2 - 170, HEIGHT - 30, 20, WHITE);
        }
        else
        {
            // game scene
            if (!gameOver)
            {
                runCountDown += dt;
                if (runCountDown >= lastUpdateCountdown)
                {
                    countdown -= 1;
                    runCountDown = 0.0;
                }

                // draw countdown
                if (countdown > -1)
                {
                    DrawText(to_string(countdown).c_str(), WIDTH / 2, 20, 30, YELLOW);
                }

                // check huruf pertama di list of meteor kata
                for (int i = 0; i < METEOR_ALIVE; i++)
                {
                    if (meteorList[i].alive)
                    {
                        if (valChar == int(meteorList[i].kata[0]) && !cek)
                        {
                            idx = i;
                            cek = true;
                        }
                    }
                }

                // check indiviual kata
                if (cek)
                {
                    if (valChar == int(meteorList[idx].kata[0]))
                    {
                        PlaySound(typingSFX);
                        wpm++;

                        meteorList[idx].kata.erase(0, 1);
                        laser.active = false;

                        deltaX = meteorList[idx].rect.x - gunPos.x;
                        deltaY = meteorList[idx].rect.y - gunPos.y;

                        angle = atan2f(deltaY, deltaX);

                        deg = angle * RAD2DEG;
                    }

                }

                if (laser.active)
                {
                    laser.resetPosition(Vector2{ gunPos.x, gunPos.y });
                }

                if (!laser.active)
                {
                    laser.speedX = cos(deg * DEG2RAD) * 35.0;
                    laser.speedY = -sin(deg * DEG2RAD) * 35.0;
                    laser.shoot();
                }

                // update meteor pos
                for (int i = 0; i < METEOR_ALIVE; i++)
                {
                    if (meteorList[i].alive)
                    {
                        // cout << "index ke i = " << i << " explode = " << meteorList[i].explode << endl;

                        Vector2 move = Vector2MoveTowards(Vector2{ meteorList[i].rect.x, meteorList[i].rect.y }, Vector2{ gunPos.x, gunPos.y }, 1.0);
                        meteorList[i].rect.x = move.x;
                        meteorList[i].rect.y = move.y;
                    }

                    if (!meteorList[i].alive)

                    {
                        cek = false;
                        meteorList[i].alive = true;
                        meteorList[i].kata = arr[GetRandomValue(0, x - 1)];
                        meteorList[i].rect.x = 610;
                        meteorList[i].rect.y = GetRandomValue(10, 450);
                        meteorList[i].live = meteorList[i].kata.length();
                    }
                }

                // check collision meteor dengan canon
                for (int i = 0; i < METEOR_ALIVE; i++)
                {
                    if (meteorList[i].alive)
                    {
                        if (CheckCollisionRecs(meteorList[i].rect, canonRect) || countdown < 0)
                        {
                            gameOver = true;
                        }
                    }
                }

                // check collision peluru dengan meteor
                if (meteorList[idx].alive)
                {
                    if (CheckCollisionRecs(laser.rect, meteorList[idx].rect) || (laser.rect.x > WIDTH || laser.rect.y > HEIGHT))
                    {

                        meteorList[idx].rect.x += 10;
                        laser.active = true;

                        meteorList[idx].explode = true;
                    }
                }

                if (meteorList[idx].kata.empty())
                {
                    if (CheckCollisionRecs(laser.rect, meteorList[idx].rect))
                    {
                        PlaySound(explosionSfx);
                        meteorList[idx].alive = false;
                        meteorList[idx].lastPos.x = meteorList[idx].rect.x;
                        meteorList[idx].lastPos.y = meteorList[idx].rect.y;
                    }
                }

                // draw canon
                DrawTextureEx(canon, canonPos, 0.0, 0.5, WHITE);
                // draw gun canon
                DrawTexturePro(gun, sourceRec, gunPos, Vector2{ 0.0, (float)gun.height / 2 }, deg, WHITE);
                // draw bullet canon
                DrawTexturePro(laser.img, laserSource, laser.rect, Vector2{ 0.0, (float)gun.height / 2 }, deg, WHITE);

                // draw list of meteor
                rotation++;
                for (int i = 0; i < METEOR_ALIVE; i++)
                {
                    if (meteorList[i].alive)
                    {

                        DrawTexturePro(meteorList[i].img, meteorList[i].meteorSource, meteorList[i].rect, Vector2{ (float)meteorList[i].img.width / 2, (float)meteorList[i].img.height / 2 }, rotation, WHITE);

                        DrawText(meteorList[i].kata.c_str(), meteorList[i].rect.x - meteorList[i].img.width / 2, meteorList[i].rect.y + meteorList[i].img.height, 20, WHITE);

                        if (cek)
                        {
                            DrawText(meteorList[idx].kata.c_str(), meteorList[idx].rect.x - meteorList[idx].img.width / 2, meteorList[idx].rect.y + meteorList[idx].img.height, 20, ORANGE);
                        }
                    }
                }

                // draw meteor explode

                if (meteorList[idx].explode)
                {
                    regularExpRect.x = meteorList[idx].rect.x;
                    regularExpRect.y = meteorList[idx].rect.y;

                    DrawTextureEx(regularExp[regularCurrentFrame], Vector2{ regularExpRect.x - meteorList[idx].img.width / 2, regularExpRect.y - meteorList[idx].img.height / 2 }, 0.0, 0.5, WHITE);
                }


                // meteor explode animation
                regularRunningTime += dt;

                if (regularRunningTime >= regularLastUpdate)
                {
                    if (meteorList[idx].explode)
                    {
                        regularCurrentFrame++;
                        if (regularCurrentFrame > regularExpFrame)
                        {
                            regularCurrentFrame = 0;
                            meteorList[idx].explode = false;
                        }
                    }
                    regularRunningTime = 0.0;
                }
            }
            else
            {
                // game over scene
                int result = (wpm / 5) / 1;
                string convertResult = to_string(result);
                string hasil = "Your WPM = " + convertResult + " WPM";

                DrawText("Game over", WIDTH / 2 - 80, HEIGHT / 4, 30, YELLOW);
                DrawText(hasil.c_str(), WIDTH / 2 - 100, HEIGHT / 2 - 50, 20, WHITE);
                DrawText("Press \"R\" for restart game", WIDTH / 2 - 135, HEIGHT / 2 + 50, 20, WHITE);
            }
        }

        // draw helper mouse position
        // string pos = "X = " + to_string(GetMouseX()) + ", Y = " + to_string(GetMouseY());
        // DrawText(pos.c_str(), GetMouseX(), GetMouseY(), 20, WHITE);

        EndDrawing();
    }
    UnloadTexture(gun);
    UnloadTexture(background);
    UnloadTexture(canon);
    UnloadImage(laserRotate);
    UnloadImage(gunRotate);
    UnloadSound(explosionSfx);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}