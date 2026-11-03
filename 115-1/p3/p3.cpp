#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <cmath>

struct Point { float x, y; };
TTF_Font* font = nullptr;

void DrawText(SDL_Renderer* r, const std::string& str, float x, float y, SDL_Color color) {
    if (str.empty()) return;
    SDL_Surface* surf = TTF_RenderText_Blended(font, str.c_str(), 0, color);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_FRect dst = { x, y, (float)surf->w, (float)surf->h };
    SDL_RenderTexture(r, tex, NULL, &dst);
    SDL_DestroyTexture(tex); SDL_DestroySurface(surf);
}

// 遞迴畫實心三角形
void DrawSierpinski(SDL_Renderer* r, Point p1, Point p2, Point p3, int depth) {
    if (depth == 0) {
        SDL_FColor col = { 0.0f, 0.0f, 0.6f, 1.0f };
        SDL_Vertex v[3] = { { {p1.x, p1.y}, col, {0,0} }, { {p2.x, p2.y}, col, {0,0} }, { {p3.x, p3.y}, col, {0,0} } };
        SDL_RenderGeometry(r, NULL, v, 3, NULL, 0);
        return;
    }
    Point m12 = { (p1.x + p2.x) / 2, (p1.y + p2.y) / 2 };
    Point m23 = { (p2.x + p3.x) / 2, (p2.y + p3.y) / 2 };
    Point m31 = { (p3.x + p1.x) / 2, (p3.y + p1.y) / 2 };

    DrawSierpinski(r, p1, m12, m31, depth - 1);
    DrawSierpinski(r, m12, p2, m23, depth - 1);
    DrawSierpinski(r, m31, m23, p3, depth - 1);
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO); TTF_Init();
    SDL_Window* win = SDL_CreateWindow("Sierpinski Triangle", 900, 600, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, NULL);
    font = TTF_OpenFont("C:\\Windows\\Fonts\\msjh.ttc", 18.0f);

    std::string input = "3";
    int depth = 3;

    SDL_StartTextInput(win);
    bool running = true; SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            else if (e.type == SDL_EVENT_TEXT_INPUT) input += e.text.text;
            else if (e.type == SDL_EVENT_KEY_DOWN) {
                if (e.key.key == SDLK_BACKSPACE && !input.empty()) input.pop_back();
                if (e.key.key == SDLK_RETURN && !input.empty()) depth = std::stoi(input);
            }
        }

        SDL_SetRenderDrawColor(ren, 240, 240, 240, 255); SDL_RenderClear(ren);
        DrawText(ren, "Sierpinski Triangle | Depth d (0~6, Press Enter): " + input, 20, 20, {0,0,0,255});

        // 畫三頂點三角形
        DrawSierpinski(ren, {450, 60}, {100, 520}, {800, 520}, depth);

        int count = (int)pow(3, depth);
        DrawText(ren, "Depth: " + std::to_string(depth) + " | Small Triangles (3^d): " + std::to_string(count), 20, 550, {0,120,0,255});

        SDL_RenderPresent(ren);
    }
    TTF_CloseFont(font); SDL_DestroyRenderer(ren); SDL_DestroyWindow(win); TTF_Quit(); SDL_Quit();
    return 0;
}