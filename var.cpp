#include "var.h"

static Font myfont; // Biến toàn cục để lưu font
#define FOR(i, a, b) for (int i = (a), _b = (b); i <= _b; i++)

void LoadFonts()
{
    myfont = LoadFont("roboto.ttf"); // Tải font từ tệp
}

Font GetFont()
{
    return myfont; // Trả về font đã load
}

void UnloadFonts()
{
    UnloadFont(myfont); // Giải phóng bộ nhớ
}
