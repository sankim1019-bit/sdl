// SDL2를 이용한 GUI 기반 물고기 키우기 게임 샘플 코드 (기초 프레임 + 게임 로직 + 승리/패배 조건 + 이미지 추가)

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define NUM 6
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define FISHTANK_WIDTH 100
#define FISHTANK_HEIGHT 200

// 게임 상태 구조체 정의
typedef struct {
    int fish;
    int water;
    int isAlive; // 1: alive, 0: dead
} FishTank;

FishTank fishTanks[NUM];    // 물고기 어항 배열
int level = 1;
int position = 0;
bool running = true;
bool gameOver = false;
bool gameWin = false;
long startTime = 0;
long lastUpdateTime = 0;

SDL_Window* window = NULL;          // SDL 창
SDL_Renderer* renderer = NULL;      // SDL 렌더러
TTF_Font* font = NULL;              // 폰트
SDL_Texture* fishTexture = NULL;    // 물고기 텍스처

bool engine_init();                 // 엔진 초기화 함수
void initGame();                    // 게임 초기화 함수
void renderText(const char* text, int x, int y); // 텍스트 렌더링 함수
void renderFishTanks();             // 어항 렌더링 함수
void updateGame();                  // 게임 상태 업데이트 함수
void renderGame();                  // 게임 렌더링 함수
void cleanupGame();                 // 게임 종료 및 자원 해제 함수
void handleInput(SDL_Event* e);     // 입력 처리 함수
SDL_Texture* loadTexture(const char* path); // 텍스처 로드 함수

int main(int argc, char* argv[]) {
    // 엔진 초기화
    if (!engine_init()) {
        printf("Error initializing engine: %s\n", SDL_GetError());  // 초기화 실패 시 에러 메시지 출력
        return 1;                   // 초기화 실패 시 프로그램 종료
    }

    initGame();                     // 게임 초기화

    while (running) {               // 게임 루프
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
            handleInput(&event);
        }
        // update the game
        updateGame();
        // render the game
        renderGame();
        // update the window
        SDL_Delay(100);             // 게임 루프 간격 조절, 100ms마다 업데이트 및 렌더링
    }
    // 게임 종료 및 자원 해제
    cleanupGame();

    return 0;
}

// 함수 정의

bool engine_init()
{
    // SDL 초기화
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return 0;
    // 창 생성 및 렌더러 초기화
    window = SDL_CreateWindow("Raising Fishes", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!window)
        return 0;
    // TTF 초기화 및 폰트 로드
    if (TTF_Init() != 0)
        return 0;
    font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 20);       // 폰트 파일 경로(절대 혹은 상대) 필요
    if (!font) {
        printf("폰트 로드 실패: %s\n", TTF_GetError());
        SDL_Quit();
        return 0;
    }
    fishTexture = loadTexture("fish.bmp");          // 물고기 이미지 파일 경로 필요
    if (!fishTexture) {                             // 텍스처 로드 실패 시 에러 메시지 출력
        SDL_Quit();
        return 0;
    }
    return 1;
}

void initGame() {
    for (int i = 0; i < NUM; i++) {
        fishTanks[i].fish = 10;
        fishTanks[i].water = 100;
        fishTanks[i].isAlive = 1;
    }
    startTime = SDL_GetTicks();             // 게임 시작 시간 기록
    lastUpdateTime = startTime;             // 마지막 업데이트 시간 초기화
}

void renderGame() {                         // 게임 화면 렌더링
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);     // 배경을 검은색으로 설정
    SDL_RenderClear(renderer);                          // 화면 초기화
    renderFishTanks();                      // 어항과 물고기 렌더링

    char levelText[64];                     // 레벨 텍스트 렌더링
    sprintf_s(levelText, sizeof(levelText), "Level %d", level);     // 현재 레벨 표시
    renderText(levelText, 10, 10);          // 게임 상태 텍스트 렌더링

    SDL_RenderPresent(renderer);            // 렌더링 업데이트
}

void cleanupGame() {
    // 종료 메시지 화면
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);             // 배경을 검은색으로 설정
    SDL_RenderClear(renderer);                                  // 화면 초기화
    if (gameWin) {
        renderText("You Win! All levels completed!", 200, 200); // 승리 메시지 렌더링
    }
    else if (gameOver) {
        renderText("Game Over! All fish are dead!", 200, 200); // 게임 오버 메시지 렌더링
    }
    else {
        renderText("Game Over", 200, 200);      // 일반 게임 오버 메시지 렌더링
    }
    SDL_RenderPresent(renderer);                // 렌더링 업데이트   
    SDL_Delay(3000);                            // 메시지 표시 후 3초 대기

    SDL_DestroyTexture(fishTexture);            // 물고기 텍스처 메모리 해제
    TTF_CloseFont(font);                        // 폰트 메모리 해제
    SDL_DestroyRenderer(renderer);              // 렌더러 메모리 해제
    SDL_DestroyWindow(window);                  // 창 메모리 해제
    TTF_Quit();                                 // TTF 종료
    SDL_Quit();                                 // SDL 종료   
}

void renderText(const char* text, int x, int y) {           // 텍스트 렌더링 함수
    SDL_Color color = { 255, 255, 255 };                    // 흰색 텍스트 색상
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);         // 텍스트를 표면으로 렌더링
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface); // 표면에서 텍스처 생성
    SDL_Rect dest = { x, y, surface->w, surface->h };       // 텍스처를 화면에 렌더링, 위치와 크기 설정
    SDL_RenderCopy(renderer, texture, NULL, &dest);         // 텍스처 렌더링
    SDL_FreeSurface(surface);                               // 표면 메모리 해제
    SDL_DestroyTexture(texture);                            // 텍스처 메모리 해제
}

void renderFishTanks() {                        // 어항과 물고기 렌더링 함수
    for (int i = 0; i < NUM; i++) {
        int x = 50 + i * (FISHTANK_WIDTH + 10);
        SDL_Rect bowl = { x, 300, FISHTANK_WIDTH, FISHTANK_HEIGHT };
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // 파란색 테두리
        SDL_RenderDrawRect(renderer, &bowl);

        // 물 높이 표시
        int waterHeight = fishTanks[i].water * FISHTANK_HEIGHT / 100;
        SDL_Rect water = { x, 300 + FISHTANK_HEIGHT - waterHeight, FISHTANK_WIDTH, waterHeight };
        SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
        SDL_RenderFillRect(renderer, &water);

        // 물고기 이미지 표시
        if (fishTanks[i].isAlive) {
            SDL_Rect fishRect = { x + 20, 300 + FISHTANK_HEIGHT - waterHeight - 30, 60, 30 };
            SDL_RenderCopy(renderer, fishTexture, NULL, &fishRect);
        }

        // 물고기 크기 텍스트 출력
        char status[64];
        if (fishTanks[i].isAlive)
            sprintf_s(status, sizeof(status), "F:%d W:%d", fishTanks[i].fish, fishTanks[i].water);
        else
            sprintf_s(status, sizeof(status), "DEAD");
        renderText(status, x + 10, 520);

        // 커서 표시
        if (i == position) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // 노란색
            SDL_RenderDrawRect(renderer, &bowl);
        }
    }
}

void updateGame() {
    long currentTime = SDL_GetTicks();
    long elapsed = (currentTime - lastUpdateTime) / 1000; // 초 단위
    if (elapsed > 0) {
        int aliveCount = 0;
        for (int i = 0; i < NUM; i++) {
            if (fishTanks[i].isAlive == 1) {
                // 물 증발 및 소비
                fishTanks[i].water -= (level * (fishTanks[i].fish / 20 + 1) * elapsed);
                if (fishTanks[i].water < 0) {
                    fishTanks[i].water = 0;
                    fishTanks[i].isAlive = 0;
                }

                // 물고기 성장
                if (fishTanks[i].water > 0)
                    fishTanks[i].fish += ((fishTanks[i].water / 100 + 1) * elapsed);
                if (fishTanks[i].fish > 100) fishTanks[i].fish = 100;

                aliveCount++;
            }
        }

        if (aliveCount == 0) {
            gameOver = true;
            running = false;
        }

        // 레벨 업 조건: 시간 경과
        long totalElapsed = (currentTime - startTime) / 1000;
        if (totalElapsed / 20 > level - 1) {
            level++;
            if (level > 5) {
                level = 5;
                gameWin = true;
                running = false;
            }
        }

        lastUpdateTime = currentTime;
    }
}

void handleInput(SDL_Event* e) {
    if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
        case SDLK_j:
            if (position > 0) position--;
            break;
        case SDLK_l:
            if (position < NUM - 1) position++;
            break;
        case SDLK_k:
            if (fishTanks[position].water >= 0 && fishTanks[position].water < 100)  // 물이 0 이상 100 미만일 때만 물을 줄 수 있도록 조건 추가
                fishTanks[position].water += 5;
            if (fishTanks[position].water > 100) fishTanks[position].water = 100; // 물이 100을 초과하지 않도록 제한
            break;
        case SDLK_ESCAPE:
            running = false;
            break;
        }
    }
}

SDL_Texture* loadTexture(const char* path) {
    SDL_Surface* surface = SDL_LoadBMP(path);
    if (!surface) {
        printf("이미지 로드 실패: %s\n", SDL_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}