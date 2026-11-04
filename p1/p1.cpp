#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>

struct Step { int pass; std::vector<int> arr; };
TTF_Font* font = nullptr;

// 文字繪製輔助函式
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

// 選擇排序：紀錄每輪快照與比較次數
void SelectionSort(std::vector<int> arr, std::vector<Step>& steps, int& comps) {
    steps.clear(); comps = 0;
    int n = arr.size();
    for (int i = 0; i < n - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < n; j++) {
            comps++; // 僅統計元素間大小比較次數
            if (arr[j] < arr[minIdx]) minIdx = j;
        }
        std::swap(arr[i], arr[minIdx]);
        steps.push_back({ i + 1, arr });
    }
}

// 輸入檢查 (防呆: 1~20 個數字、0~999 範圍、非整數檢查)
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
    SDL_Window* win = SDL_CreateWindow("Selection Sort Visualizer", 920, 760, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, NULL);
    font = TTF_OpenFont("C:\\Windows\\Fonts\\msjh.ttc", 17.0f);

    std::string input = "4, 3, 2, 3";
    std::vector<int> origArr;
    std::vector<Step> steps;
    std::string err = "";
    int comps = 0;

    if (ParseInput(input, origArr, err)) SelectionSort(origArr, steps, comps);

    SDL_StartTextInput(win);
    bool running = true; SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            else if (e.type == SDL_EVENT_TEXT_INPUT) {
                if (e.text.text) {
                    for (int i = 0; e.text.text[i] != '\0'; i++) {
                        char c = e.text.text[i];
                        if (isdigit(c) || c == ',' || c == ' ') input += c;
                    }
                }
            }
            else if (e.type == SDL_EVENT_KEY_DOWN) {
                if (e.key.key == SDLK_BACKSPACE && !input.empty()) input.pop_back();
                if (e.key.key == SDLK_RETURN) {
                    if (ParseInput(input, origArr, err)) SelectionSort(origArr, steps, comps);
                }
            }
        }

        SDL_SetRenderDrawColor(ren, 245, 245, 245, 255); SDL_RenderClear(ren);

        // 1. 演算法名稱、規則標示與複雜度說明
        DrawText(ren, "【演算法】選擇排序 (Selection Sort)", 20, 15, {0, 0, 0, 255});
        DrawText(ren, "【規則說明】標準選擇排序 (固定執行 N-1 輪，每輪搜尋未排序區間之最小值進行交換)", 20, 38, {0, 100, 150, 255});
        DrawText(ren, "【複雜度】時間 O(N^2) | 額外空間 O(1)  (N 代表陣列元素個數)", 20, 61, {90, 90, 90, 255});

        // 2. 輸入框與動態閃爍游標
        bool cursor = (SDL_GetTicks() / 500) % 2;
        DrawText(ren, "輸入 (1~20個數字 0~999, Enter執行): " + input + (cursor ? "|" : " "), 20, 88, {0, 50, 180, 255});

        // 3. 錯誤提示 或 排序完整過程
        if (!err.empty()) {
            DrawText(ren, err, 20, 125, {200, 0, 0, 255});
        } else {
            // A. 原始資料
            std::string origStr = "【原始資料】: [ ";
            for (size_t i = 0; i < origArr.size(); i++) origStr += std::to_string(origArr[i]) + (i + 1 < origArr.size() ? ", " : " ");
            origStr += "]";
            DrawText(ren, origStr, 20, 120, {0, 0, 0, 255});

            // B. 每完成一輪外層迴圈後的陣列
            float y = 145;
            for (auto& s : steps) {
                std::string line = "第 " + std::to_string(s.pass) + " 輪: [ ";
                for (size_t i = 0; i < s.arr.size(); i++) line += std::to_string(s.arr[i]) + (i + 1 < s.arr.size() ? ", " : " ");
                DrawText(ren, line + "]", 35, y, {0, 100, 50, 255});
                y += 22;
            }

            // C. 最終升冪結果
            std::vector<int> finalArr = steps.empty() ? origArr : steps.back().arr;
            std::string finalStr = "【最終升冪結果】: [ ";
            for (size_t i = 0; i < finalArr.size(); i++) finalStr += std::to_string(finalArr[i]) + (i + 1 < finalArr.size() ? ", " : " ");
            finalStr += "]";
            DrawText(ren, finalStr, 20, y + 6, {0, 0, 180, 255});

            // D. 元素大小比較次數
            DrawText(ren, "【比較次數】元素大小比較總計: " + std::to_string(comps) + " 次", 20, y + 32, {180, 0, 0, 255});
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    TTF_CloseFont(font); SDL_DestroyRenderer(ren); SDL_DestroyWindow(win); TTF_Quit(); SDL_Quit();
    return 0;
}