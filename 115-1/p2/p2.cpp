#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <vector>
#include <string>
#include <sstream>

struct Step { int low, high, pivot; std::vector<int> arr; };
TTF_Font* font = nullptr;

void DrawText(SDL_Renderer* r, const std::string& str, float x, float y, SDL_Color color) {
    if (str.empty()) return;
    SDL_Surface* surf = TTF_RenderText_Blended(font, str.c_str(), 0, color);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_FRect dst = { x, y, (float)surf->w, (float)surf->h };
    SDL_RenderTexture(r, tex, NULL, &dst);
    SDL_DestroyTexture(tex); SDL_DestroySurface(surf);
}

// 快速排序 Lomuto Partition
void QuickSort(std::vector<int>& arr, int low, int high, std::vector<Step>& steps) {
    if (low < high) {
        int pivot = arr[high], i = low - 1;
        for (int j = low; j < high; j++) {
            if (arr[j] <= pivot) std::swap(arr[++i], arr[j]);
        }
        std::swap(arr[i + 1], arr[high]);
        int pi = i + 1;
        steps.push_back({ low, high, pivot, arr });

        QuickSort(arr, low, pi - 1, steps);
        QuickSort(arr, pi + 1, high, steps);
    }
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO); TTF_Init();
    SDL_Window* win = SDL_CreateWindow("Quick Sort", 900, 600, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, NULL);
    font = TTF_OpenFont("C:\\Windows\\Fonts\\msjh.ttc", 18.0f);

    std::string input = "5, 3, 8, 1, 2";
    std::vector<Step> steps;
    bool done = false;

    SDL_StartTextInput(win);
    bool running = true; SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            else if (e.type == SDL_EVENT_TEXT_INPUT) input += e.text.text;
            else if (e.type == SDL_EVENT_KEY_DOWN) {
                if (e.key.key == SDLK_BACKSPACE && !input.empty()) input.pop_back();
                if (e.key.key == SDLK_RETURN) {
                    std::vector<int> arr; std::stringstream ss(input); std::string item;
                    while (std::getline(ss, item, ',')) arr.push_back(std::stoi(item));
                    steps.clear();
                    QuickSort(arr, 0, arr.size() - 1, steps);
                    done = true;
                }
            }
        }

        SDL_SetRenderDrawColor(ren, 240, 240, 240, 255); SDL_RenderClear(ren);
        DrawText(ren, "Quick Sort | Input (Enter to sort): " + input, 20, 20, {0,0,0,255});

        if (done) {
            float y = 60;
            for (size_t k = 0; k < steps.size(); k++) {
                auto& s = steps[k];
                std::string line = "Step " + std::to_string(k+1) + " [" + std::to_string(s.low) + ".." + std::to_string(s.high) + "] Pivot=" + std::to_string(s.pivot) + " -> [";
                for (size_t i = 0; i < s.arr.size(); i++) line += std::to_string(s.arr[i]) + (i+1<s.arr.size()?", ":"");
                DrawText(ren, line + "]", 30, y, {0,0,150,255});
                y += 25;
            }
            DrawText(ren, "Complexity: Time Avg O(N log N) Worst O(N^2) | Space O(log N)", 20, y + 20, {0,100,0,255});
        }
        SDL_RenderPresent(ren);
    }
    TTF_CloseFont(font); SDL_DestroyRenderer(ren); SDL_DestroyWindow(win); TTF_Quit(); SDL_Quit();
    return 0;
}