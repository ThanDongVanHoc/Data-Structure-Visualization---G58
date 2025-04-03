#include "var.h"

static Font myfont; // Biến toàn cục để lưu font

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
