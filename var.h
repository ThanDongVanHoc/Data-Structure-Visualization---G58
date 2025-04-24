#ifndef VAR_H
#define VAR_H

#include "raylib.h"
#include "trie.h"
void LoadFonts();   // Hàm để tải font khi khởi động
Font GetFont();     // Hàm để lấy font trong các file khác
void UnloadFonts(); // Giải phóng bộ nhớ khi không cần nữa

#endif
