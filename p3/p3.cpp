#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <sstream>
#include <cctype>
#include <cmath>

struct Point { float x, y; };
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

// 實心填色繪製
void DrawSierpinski(SDL_Renderer* r, int depth, Point p1, Point p2, Point p3) {
    if (depth == 0) {
        SDL_FColor col = { 0.0f, 0.51f, 0.59f, 1.0f }; // 藍綠色填滿
        SDL_Vertex verts[3] = {
            { { p1.x, p1.y }, col, { 0.0f, 0.0f } },
            { { p2.x, p2.y }, col, { 0.0f, 0.0f } },
            { { p3.x, p3.y }, col, { 0.0f, 0.0f } }
        };
        SDL_RenderGeometry(r, NULL, verts, 3, NULL, 0);
        return;
    }
    Point m1 = { (p1.x + p2.x) / 2.0f, (p1.y + p2.y) / 2.0f };
    Point m2 = { (p2.x + p3.x) / 2.0f, (p2.y + p3.y) / 2.0f };
    Point m3 = { (p3.x + p1.x) / 2.0f, (p3.y + p1.y) / 2.0f };

    DrawSierpinski(r, depth - 1, p1, m1, m3);
    DrawSierpinski(r, depth - 1, m1, p2, m2);
    DrawSierpinski(r, depth - 1, m3, m2, p3);
}

// 依 PDF 規定：深度限制在 0 至 6
bool ParseDepth(const std::string& str, int& depth, std::string& err) {
    std::stringstream ss(str);
    std::string token, extra;
    if (!(ss >> token)) { err = "錯誤: 輸入不得為空 (請輸入 0~6 的整數)"; return false; }
    if (ss >> extra) { err = "錯誤: 格式錯誤，僅能輸入單一整數"; return false; }
    for (char c : token) {
        if (!isdigit(c)) { err = "錯誤: 包含非數字字元 '" + token + "'"; return false; }
    }
    try {
        int d = std::stoi(token);
        if (d < 0 || d > 6) { err = "錯誤: 遞迴深度 " + token + " 超出範圍 (須為 0~6)"; return false; }
        depth = d;
    } catch (...) {
        err = "錯誤: 數字解析失敗";
        return false;
    }
    err = "";
    return true;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO); TTF_Init();
    SDL_Window* win = SDL_CreateWindow("Sierpinski Triangle Visualizer", 920, 760, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, NULL);
    font = TTF_OpenFont("C:\\Windows\\Fonts\\msjh.ttc", 17.0f);

    std::string input = "4";
    int depth = 4;
    std::string err = "";

    ParseDepth(input, depth, err);

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
                    ParseDepth(input, depth, err);
                }
            }
        }

        SDL_SetRenderDrawColor(ren, 245, 245, 245, 255); SDL_RenderClear(ren);

        DrawText(ren, "【演算法】謝爾賓斯基三角形 (Sierpinski Triangle)", 20, 15, {0, 0, 0, 255});
        DrawText(ren, "【規則說明】分形遞迴繪製 (d=0 繪製實心正三角形，d>0 遞迴去除中央倒三角形)", 20, 38, {0, 100, 150, 255});
        DrawText(ren, "【複雜度】時間 O(3^d) | 額外空間 O(d) 遞迴堆疊  (d 代表遞迴階數 Depth)", 20, 61, {90, 90, 90, 255});

        bool cursor = (SDL_GetTicks() / 500) % 2;
        DrawText(ren, "【輸入深度】(0~6整數, Enter執行): " + input + (cursor ? "|" : " "), 20, 88, {0, 50, 180, 255});

        if (!err.empty()) {
            DrawText(ren, err, 20, 120, {200, 0, 0, 255});
        } else {
            int count = (int)std::pow(3, depth);
            // 完全對齊 PDF 指定名稱：「最末層保留的小三角形總數」
            std::string infoStr = "【當前深度】d = " + std::to_string(depth) + " | 【最末層保留的小三角形總數 (3^d)】: " + std::to_string(count) + " 個";
            DrawText(ren, infoStr, 20, 120, {0, 100, 50, 255});

            Point p1 = { 460.0f, 160.0f };
            Point p2 = { 60.0f,  720.0f };
            Point p3 = { 860.0f, 720.0f };

            DrawSierpinski(ren, depth, p1, p2, p3);
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    TTF_CloseFont(font); SDL_DestroyRenderer(ren); SDL_DestroyWindow(win); TTF_Quit(); SDL_Quit();
    return 0;
}