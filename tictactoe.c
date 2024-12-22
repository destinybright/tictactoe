#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 600
#define CELL_SIZE 200

char board[3][3];
char currentPlayer = 'X';
Uint32 compTurnStartTime = 0;
bool compTurnPending = false;
bool gameOver = false;
char gameResult[50];

SDL_Texture* xTexture = NULL;
SDL_Texture* oTexture = NULL;

SDL_Texture* loadTexture(const char* file, SDL_Renderer* renderer) {
    SDL_Surface* tempSurface = IMG_Load(file);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    return texture;
}

void initializeBoard() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            board[i][j] = ' ';
        }
    }
}

void drawBoard(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 176, 0, 81, 255);
    for (int i = 1; i < 3; i++) {
        SDL_RenderDrawLine(renderer, i * CELL_SIZE, 0, i * CELL_SIZE, WINDOW_HEIGHT);
        SDL_RenderDrawLine(renderer, 0, i * CELL_SIZE, WINDOW_WIDTH, i * CELL_SIZE);
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            SDL_Rect cell = { j * CELL_SIZE, i * CELL_SIZE, CELL_SIZE, CELL_SIZE };

            if (board[i][j] == 'X') {
                SDL_RenderCopy(renderer, xTexture, NULL, &cell);
            } else if (board[i][j] == 'O') {
                SDL_RenderCopy(renderer, oTexture, NULL, &cell);
            }
        }
    }
}

void drawGameOverOverlay(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 254, 171, 185, 255);
    SDL_Rect overlay = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    SDL_RenderFillRect(renderer, &overlay);

    SDL_Color textColor = { 176, 0, 81, 255 };
    TTF_Font* font = TTF_OpenFont("font.TTF", 100);
    if (font) {
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, gameResult, textColor);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_FreeSurface(textSurface);

            if (textTexture) {
                SDL_Rect textRect = { 100, WINDOW_HEIGHT / 2 - 50, WINDOW_WIDTH - 200, 100 };
                SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                SDL_DestroyTexture(textTexture);
            }
        }
        TTF_CloseFont(font);
    }
}

bool checkWin(char symbol) {
    for (int i = 0; i < 3; i++) {
        if ((board[i][0] == symbol && board[i][1] == symbol && board[i][2] == symbol) ||
            (board[0][i] == symbol && board[1][i] == symbol && board[2][i] == symbol)) {
            return true;
        }
    }

    if ((board[0][0] == symbol && board[1][1] == symbol && board[2][2] == symbol) ||
        (board[0][2] == symbol && board[1][1] == symbol && board[2][0] == symbol)) {
        return true;
    }
    return false;
}

bool isBoardFull() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == ' ') {
                return false;
            }
        }
    }
    return true;
}

void compMove() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == ' ') {
                board[i][j] = 'O';
                return;
            }
        }
    }
}

void handleClick(int x, int y) {
    int row = y / CELL_SIZE;
    int col = x / CELL_SIZE;

    if (board[row][col] == ' ' && currentPlayer == 'X' && !gameOver) {
        board[row][col] = currentPlayer;

        if (checkWin(currentPlayer)) {
            snprintf(gameResult, sizeof(gameResult), "You Won!");
            gameOver = true;
        } else if (isBoardFull()) {
            snprintf(gameResult, sizeof(gameResult), "It's a Draw!");
            gameOver = true;
        } else {
            compTurnPending = true;
            compTurnStartTime = SDL_GetTicks() + 800;
        }
    }
}

void compTurn() {
    if (!gameOver) {
        compMove();

        if (checkWin('O')) {
            snprintf(gameResult, sizeof(gameResult), "You Lost!");
            gameOver = true;
        } else if (isBoardFull()) {
            snprintf(gameResult, sizeof(gameResult), "It's a Draw!");
            gameOver = true;
        } else {
            currentPlayer = 'X';
        }
    }
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Tic Tac Toe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    xTexture = loadTexture("x.png", renderer);
    oTexture = loadTexture("o.png", renderer);

    if (!xTexture || !oTexture) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    initializeBoard();

    bool running = true;
    SDL_Event event;

    while (running) {
        SDL_SetRenderDrawColor(renderer, 254, 171, 185, 255);
        SDL_RenderClear(renderer);

        drawBoard(renderer);

        if (gameOver) {
            drawGameOverOverlay(renderer);
        }

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_MOUSEBUTTONDOWN && currentPlayer == 'X') {
                handleClick(event.button.x, event.button.y);
            }
        }

        if (compTurnPending && SDL_GetTicks() >= compTurnStartTime) {
            compTurn();
            compTurnPending = false;
        }
    }

    SDL_DestroyTexture(xTexture);
    SDL_DestroyTexture(oTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
