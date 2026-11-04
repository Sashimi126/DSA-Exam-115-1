#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>

struct Step {
    int pass;
    std::string info;
    std::vector<int> arr;
};

TTF_Font* font = nullptr;

void DrawText(SDL_Renderer* r, const std::string& str, float x, float y, SDL_Color col) {
    if (str.empty()) return;
    SDL_Surface* surf = TTF_RenderText_Blended(font, str.c_str(), 0, col);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_FRect dst = { x, y, (float)surf->w, (float)surf->h };
    SDL_RenderTexture(r, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_DestroySurface(surf);
}

int Partition(std::vector<int>& arr, int low, int high, std::vector<Step>& steps, int& comps, int& stepCounter) {
    int pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; j++) {
        comps++;
        if (arr[j] < pivot) {
            i++;
            std::swap(arr[i], arr[j]);
        }
    }
    std::swap(arr[i + 1], arr[high]);
    stepCounter++;
    // 依 PDF 規範明確標示「處理區間 [low..high]」與「Pivot 值」
    std::string detail = "處理區間 [" + std::to_string(low) + ".." + std::to_string(high) + "], Pivot " + std::to_string(pivot) + " 歸位至索引 " + std::to_string(i + 1);
    steps.push_back({ stepCounter, detail, arr });
    return i + 1;
}

void QuickSortHelper(std::vector<int>& arr, int low, int high, std::vector<Step>& steps, int& comps, int& stepCounter) {
    if (low < high) {
        int pi = Partition(arr, low, high, steps, comps, stepCounter);
        QuickSortHelper(arr, low, pi - 1, steps, comps, stepCounter);
        QuickSortHelper(arr, pi + 1, high, steps, comps, stepCounter);
    }
}

void QuickSort(std::vector<int> arr, std::vector<Step>& steps, int& comps) {
    steps.clear(); comps = 0;
    int stepCounter = 0;
    if (!arr.empty()) {
        QuickSortHelper(arr, 0, arr.size() - 1, steps, comps, stepCounter);
    }
}

bool ParseInput(const std::string& str, std::vector<int>& arr, std::string& err) {
    arr.clear();
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, ',')) {
        std::stringstream tss(token);
        std::string sub, extra;
        if (!(tss >> sub)) continue;
        if (tss >> extra) { err = "錯誤: 格式混亂 (" + token + ")"; return false; }
        for (char c : sub) {
            if (!isdigit(c)) { err = "錯誤: 包含非數字字元 '" + sub + "'"; return false; }
        }
        int v = std::stoi(sub);
        if (v < 0 || v > 999) { err = "錯誤: 數值 " + sub + " 超出 0~999 範圍"; return false; }
        arr.push_back(v);
    }
    if (arr.empty()) { err = "錯誤: 輸入不得為空 (請輸入 1~20 個數字)"; return false; }
    if (arr.size() > 20) { err = "錯誤: 數量 " + std::to_string(arr.size()) + " 個 (超出上限 20 個)"; return false; }
    err = "";
    return true;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO); TTF_Init();
    SDL_Window* win = SDL_CreateWindow("Quick Sort Visualizer", 920, 760, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, NULL);
    font = TTF_OpenFont("C:\\Windows\\Fonts\\msjh.ttc", 17.0f);

    std::string input = "38, 27, 43, 3, 9, 82, 10";
    std::vector<int> origArr;
    std::vector<Step> steps;
    std::string err = "";
    int comps = 0;

    if (ParseInput(input, origArr, err)) QuickSort(origArr, steps, comps);

    SDL_StartTextInput(win);
    bool running = true; SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            else if (e.type == SDL_EVENT_TEXT_INPUT) {
                if (e.text.text) {
                    for (int i = 0; e.text.text[i] != '\0'; i++) {
                        char c = e.text.text[i];
                        if (c >= 32 && c <= 126) input += c;
                    }
                }
            }
            else if (e.type == SDL_EVENT_KEY_DOWN) {
                if (e.key.key == SDLK_BACKSPACE && !input.empty()) input.pop_back();
                if (e.key.key == SDLK_RETURN) {
                    if (ParseInput(input, origArr, err)) QuickSort(origArr, steps, comps);
                }
            }
        }

        SDL_SetRenderDrawColor(ren, 245, 245, 245, 255); SDL_RenderClear(ren);

        DrawText(ren, "【演算法】快速排序 (Quick Sort)", 20, 15, {0, 0, 0, 255});
        DrawText(ren, "【規則說明】Lomuto Partition (選末尾為 Pivot，小於 Pivot 置於左側完成分割歸位)", 20, 38, {0, 100, 150, 255});
        DrawText(ren, "【複雜度】時間 平均 O(N log N) / 最壞 O(N^2) | 額外空間 O(log N)  (N 代表陣列元素個數)", 20, 61, {90, 90, 90, 255});

        bool cursor = (SDL_GetTicks() / 500) % 2;
        DrawText(ren, "【輸入資料】(1~20個數字 0~999, Enter執行): " + input + (cursor ? "|" : " "), 20, 88, {0, 50, 180, 255});

        if (!err.empty()) {
            DrawText(ren, err, 20, 125, {200, 0, 0, 255});
        } else {
            std::string origStr = "【原始資料】: [ ";
            for (size_t i = 0; i < origArr.size(); i++) origStr += std::to_string(origArr[i]) + (i + 1 < origArr.size() ? ", " : " ");
            origStr += "]";
            DrawText(ren, origStr, 20, 120, {0, 0, 0, 255});

            float y = 145;
            for (auto& s : steps) {
                std::string line = "階段 " + std::to_string(s.pass) + " (" + s.info + "): [ ";
                for (size_t i = 0; i < s.arr.size(); i++) line += std::to_string(s.arr[i]) + (i + 1 < s.arr.size() ? ", " : " ");
                DrawText(ren, line + "]", 35, y, {0, 100, 50, 255});
                y += 22;
            }

            std::vector<int> finalArr = steps.empty() ? origArr : steps.back().arr;
            std::string finalStr = "【最終升冪結果】: [ ";
            for (size_t i = 0; i < finalArr.size(); i++) finalStr += std::to_string(finalArr[i]) + (i + 1 < finalArr.size() ? ", " : " ");
            finalStr += "]";
            DrawText(ren, finalStr, 20, y + 6, {0, 0, 180, 255});

            DrawText(ren, "【比較次數】元素大小比較總計: " + std::to_string(comps) + " 次", 20, y + 32, {180, 0, 0, 255});
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    TTF_CloseFont(font); SDL_DestroyRenderer(ren); SDL_DestroyWindow(win); TTF_Quit(); SDL_Quit();
    return 0;
}