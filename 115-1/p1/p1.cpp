#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <vector>
#include <string>
#include <sstream>

struct Step { int pass; std::vector<int> arr; };
TTF_Font* font = nullptr;

// 文字渲染極簡函式
void DrawText(SDL_Renderer* r, const std::string& str, float x, float y, SDL_Color color) {
    if (str.empty()) return;
    SDL_Surface* surf = TTF_RenderText_Blended(font, str.c_str(), 0, color);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_FRect dst = { x, y, (float)surf->w, (float)surf->h };
    SDL_RenderTexture(r, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_DestroySurface(surf);
}

// 選擇排序核心 (計算比較次數與記錄步驟)
void SelectionSort(std::vector<int> arr, std::vector<Step>& steps, int& comps) {
    steps.clear(); comps = 0;
    int n = arr.size();
    for (int i = 0; i < n - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < n; j++) {
            comps++; // 統計比較次數
            if (arr[j] < arr[minIdx]) minIdx = j;
        }
        std::swap(arr[i], arr[minIdx]);
        steps.push_back({ i + 1, arr });
    }
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO); TTF_Init();
    SDL_Window* win = SDL_CreateWindow("Selection Sort", 900, 600, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, NULL);
    font = TTF_OpenFont("C:\\Windows\\Fonts\\msjh.ttc", 18.0f);

    std::string input = "5, 3, 8, 1, 2";
    std::vector<Step> steps;
    int comps = 0;
    bool done = false;

    SDL_StartTextInput(win);
    bool running = true; SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            else if (e.type == SDL_EVENT_TEXT_INPUT) input += e.text.text;
            else if (e.type == SDL_EVENT_KEY_DOWN) {
                if (e.key.key == SDLK_BACKSPACE && !input.empty()) input.pop_back();
                if (e.key.key == SDLK_RETURN) { // Enter 送出
                    std::vector<int> arr; std::stringstream ss(input); std::string item;
                    while (std::getline(ss, item, ',')) arr.push_back(std::stoi(item));
                    SelectionSort(arr, steps, comps);
                    done = true;
                }
            }
        }

        SDL_SetRenderDrawColor(ren, 240, 240, 240, 255); SDL_RenderClear(ren);

        DrawText(ren, "Selection Sort | Input (Enter to sort): " + input, 20, 20, {0,0,0,255});

        if (done) {
            float y = 60;
            for (auto& s : steps) {
                std::string line = "Pass " + std::to_string(s.pass) + ": [";
                for (size_t i = 0; i < s.arr.size(); i++) line += std::to_string(s.arr[i]) + (i+1<s.arr.size()?", ":"");
                DrawText(ren, line + "]", 30, y, {0,0,150,255});
                y += 25;
            }
            DrawText(ren, "Comparisons: " + std::to_string(comps), 20, y + 10, {150,0,0,255});
            DrawText(ren, "Complexity: Time O(N^2), Space O(1)", 20, y + 40, {0,100,0,255});
        }
        SDL_RenderPresent(ren);
    }
    TTF_CloseFont(font); SDL_DestroyRenderer(ren); SDL_DestroyWindow(win); TTF_Quit(); SDL_Quit();
    return 0;
}