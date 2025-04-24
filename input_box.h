#ifndef INPUT_BOX_H
#define INPUT_BOX_H

#include "raylib.h"
#include <string>
#include <vector>
#include <stack>

// Kích thước giao diện và font
extern const int screenWidth;
extern const int screenHeight;
extern const int fontSize;
extern const float lineHeight;
extern const int padding;

// Hộp soạn thảo (input box)
extern Rectangle inputBox;

// Nội dung văn bản
extern std::vector<std::string> lines;
extern int cursorX, cursorY;

// Cuộn nội dung theo trục Y
extern float scrollOffset;

// Biến hỗ trợ vùng chọn văn bản
extern bool isSelecting;
extern bool selectionActive;
extern int selStartX, selStartY, selEndX, selEndY;

// Cấu trúc lưu trạng thái của editor (Undo/Redo)
struct EditorState
{
    std::vector<std::string> lines;
    int cursorX, cursorY;
    bool selectionActive;
    int selStartX, selStartY, selEndX, selEndY;
};

extern std::stack<EditorState> undoStack;
extern std::stack<EditorState> redoStack;

// Hàm xử lý file
void SaveFile();
void OpenFile();
void NewFile();

// Hàm Undo/Redo
void SaveStateForUndo();
void RestoreState(const EditorState &state);

// Hàm xử lý vùng chọn
void DeleteSelection();
void DrawSelectionHighlight(Rectangle inputBox);
// Hàm chuyển từ tọa độ chuột sang (line, char)
void GetCursorFromMousePos(const Vector2 &mousePos, int &outLine, int &outChar, Rectangle inputBox);
std::vector<std::string> HandleInputGraphUI(Rectangle inputBox, bool &inputGraphActive, int screenWidth, int screenHeight);
#endif // INPUT_BOX_H