#include "raylib.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stack>

// Cài đặt kích thước giao diện và font
const int screenWidth = 1000;
const int screenHeight = 600;
const int fontSize = 20;
const float lineHeight = fontSize * 1.2f;
const int padding = 5;

// Nội dung văn bản: mỗi phần tử là một dòng
std::vector<std::string> lines = {""};
// Vị trí con trỏ: (cursorX, cursorY)
int cursorX = 0, cursorY = 0;

// Cuộn nội dung theo trục Y
float scrollOffset = 0;

//--- Các biến hỗ trợ vùng chọn văn bản ---
// Khi đang chọn, isSelecting = true
bool isSelecting = false;
// Vùng chọn được lưu theo dạng tọa độ: (selStartX, selStartY) & (selEndX, selEndY)
// Ghi nhớ rằng các giá trị này là chỉ số ký tự trong dòng và số thứ tự dòng.
int selStartX = 0, selStartY = 0;
int selEndX = 0, selEndY = 0;

// Chọn toàn bộ (Ctrl+A) cũng thiết lập isSelecting = true và vùng chọn từ đầu đến cuối.
bool selectionActive = false; // Nếu có vùng chọn đang "có hiệu lực"

// Cấu trúc lưu trạng thái của editor (Undo/Redo)
struct EditorState
{
    std::vector<std::string> lines;
    int cursorX, cursorY;
    bool selectionActive;
    int selStartX, selStartY, selEndX, selEndY;
};

std::stack<EditorState> undoStack;
std::stack<EditorState> redoStack;

// Hàm nối tất cả các dòng thành 1 chuỗi với '\n'
std::string GetAllText(const std::vector<std::string> &lines)
{
    std::ostringstream oss;
    for (size_t i = 0; i < lines.size(); i++)
    {
        oss << lines[i];
        if (i < lines.size() - 1)
            oss << "\n";
    }
    return oss.str();
}

void NewFile()
{
    lines.clear();
    lines.push_back("");
    cursorX = 0;
    cursorY = 0;
}

// Lưu trạng thái để hỗ trợ Undo/Redo
void SaveStateForUndo()
{
    EditorState state{lines, cursorX, cursorY, selectionActive, selStartX, selStartY, selEndX, selEndY};
    undoStack.push(state);
    while (!redoStack.empty())
        redoStack.pop();
}

void RestoreState(const EditorState &state)
{
    lines = state.lines;
    cursorX = state.cursorX;
    cursorY = state.cursorY;
    selectionActive = state.selectionActive;
    selStartX = state.selStartX;
    selStartY = state.selStartY;
    selEndX = state.selEndX;
    selEndY = state.selEndY;
}

// Xóa vùng chọn hiện có (ở đây, kiểu đơn giản: xóa nội dung được chọn)
void DeleteSelection()
{
    // Với ví dụ này, nếu vùng chọn đang có hiệu lực, ta xóa nội dung trong vùng chọn.
    // Ta sẽ xử lý theo thứ tự dòng.
    if (!selectionActive)
        return;

    // Sắp xếp lại vùng chọn: (start) và (end) sao cho (start) trước (end).
    int startY = selStartY, endY = selEndY;
    int startX = selStartX, endX = selEndX;
    if (startY > endY || (startY == endY && startX > endX))
    {
        std::swap(startY, endY);
        std::swap(startX, endX);
    }

    SaveStateForUndo();
    if (startY == endY)
    {
        // Xóa phần trong 1 dòng
        lines[startY].erase(startX, endX - startX);
        cursorY = startY;
        cursorX = startX;
    }
    else
    {
        // Xóa phần từ dòng đầu cho đến dòng cuối
        // Phần đầu của dòng đầu tiên đến cuối vùng chọn
        std::string firstPart = lines[startY].substr(0, startX);
        // Phần cuối của dòng cuối cùng sau vùng chọn
        std::string lastPart = lines[endY].substr(endX);
        // Nối lại
        lines[startY] = firstPart + lastPart;
        // Xóa các dòng nằm giữa
        for (int i = endY; i > startY; i--)
        {
            lines.erase(lines.begin() + i);
        }
        cursorY = startY;
        cursorX = startX;
    }
    selectionActive = false;
    isSelecting = false;
}

//////////////////////
// Hàm vẽ vùng chọn //
//////////////////////
void DrawSelectionHighlight(Rectangle inputBox)
{
    if (!selectionActive)
        return;

    // Sắp xếp lại vùng chọn: (start) và (end) sao cho (start) trước (end)
    int startY = selStartY, endY = selEndY;
    int startX = selStartX, endX = selEndX;
    if (startY > endY || (startY == endY && startX > endX))
    {
        std::swap(startY, endY);
        std::swap(startX, endX);
    }

    // Dùng màu xanh (alpha giảm nhẹ)
    Color highlightColor = {100, 149, 237, 100}; // CornflowerBlue với alpha = 100

    // Vẽ cho từng dòng từ startY đến endY
    for (int lineIndex = startY; lineIndex <= endY; lineIndex++)
    {
        float y = inputBox.y + lineIndex * lineHeight - scrollOffset;
        // Nếu dòng không nằm trong vùng input, bỏ qua
        if (y + lineHeight < inputBox.y || y > inputBox.y + inputBox.height)
            continue;

        int lineLen = (int)lines[lineIndex].size();
        int sX = 0, eX = 0;
        if (lineIndex == startY && lineIndex == endY)
        {
            // Nếu vùng chọn nằm trong cùng một dòng
            sX = startX;
            eX = endX;
        }
        else if (lineIndex == startY)
        {
            sX = startX;
            eX = lineLen;
        }
        else if (lineIndex == endY)
        {
            sX = 0;
            eX = endX;
        }
        else
        {
            sX = 0;
            eX = lineLen;
        }
        // Tính vị trí X dựa trên MeasureText
        int highlightStartX = inputBox.x + padding + MeasureText(lines[lineIndex].substr(0, sX).c_str(), fontSize);
        int highlightEndX = inputBox.x + padding + MeasureText(lines[lineIndex].substr(0, eX).c_str(), fontSize);
        // Vẽ hình chữ nhật vùng chọn
        Rectangle rect = {(float)highlightStartX, y, (float)(highlightEndX - highlightStartX), lineHeight};
        DrawRectangleRec(rect, highlightColor);
    }
}

//////////////////////////////
// Hàm chuyển từ tọa độ chuột sang (line, char)
//////////////////////////////
void GetCursorFromMousePos(const Vector2 &mousePos, int &outLine, int &outChar, Rectangle inputBox)
{
    // Tính vị trí theo trục Y
    float relativeY = mousePos.y - inputBox.y + scrollOffset;
    int lineIndex = (int)(relativeY / lineHeight);
    if (lineIndex < 0)
        lineIndex = 0;
    if (lineIndex >= (int)lines.size())
        lineIndex = lines.size() - 1;
    outLine = lineIndex;

    // Tính vị trí X trong dòng
    float relativeX = mousePos.x - inputBox.x - padding;
    int charIndex = 0;
    while (charIndex <= (int)lines[lineIndex].size())
    {
        int width = MeasureText(lines[lineIndex].substr(0, charIndex).c_str(), fontSize);
        if (width > relativeX)
            break;
        charIndex++;
    }
    if (charIndex > (int)lines[lineIndex].size())
        charIndex = lines[lineIndex].size();
    outChar = charIndex;
}

//////////////////////////////
// Main
//////////////////////////////

std::vector<std::string> HandleInputGraphUI(Rectangle inputBox, bool &inputGraphActive, int screenWidth, int screenHeight)
{
    static int frameCounter = 0; // Đếm khung hình để nhấp nháy con trỏ

    // ===== XỬ LÝ TÀI LỆNH FILE (Ctrl+S, O, N) =====
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
    {
        if (IsKeyPressed(KEY_N))
        {
            SaveStateForUndo();
            NewFile();
        }
    }

    // ===== XỬ LÝ UNDO/REDO =====
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))
    {
        if (IsKeyPressed(KEY_Z))
        { // Undo
            if (!undoStack.empty())
            {
                EditorState curState{lines, cursorX, cursorY, selectionActive, selStartX, selStartY, selEndX, selEndY};
                redoStack.push(curState);
                EditorState prevState = undoStack.top();
                undoStack.pop();
                RestoreState(prevState);
            }
        }
        if (IsKeyPressed(KEY_Y))
        { // Redo
            if (!redoStack.empty())
            {
                EditorState curState{lines, cursorX, cursorY, selectionActive, selStartX, selStartY, selEndX, selEndY};
                undoStack.push(curState);
                EditorState nextState = redoStack.top();
                redoStack.pop();
                RestoreState(nextState);
            }
        }
    }

    // ===== XỬ LÝ CHỌN TOÀN BỘ (Ctrl+A) =====
    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_A))
    {
        selectionActive = true;
        isSelecting = false; // không phải chọn bằng chuột
        selStartX = 0;
        selStartY = 0;
        selEndY = lines.size() - 1;
        selEndX = (int)lines.back().size();
        cursorX = selEndX;
        cursorY = selEndY;
    }

    // ===== XỬ LÝ COPY, CUT, PASTE =====
    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_C))
    {
        if (selectionActive)
        {
            // Copy vùng chọn
            int startY = selStartY, endY = selEndY, startX = selStartX, endX = selEndX;
            if (startY > endY || (startY == endY && startX > endX))
            {
                std::swap(startY, endY);
                std::swap(startX, endX);
            }
            std::ostringstream oss;
            for (int i = startY; i <= endY; i++)
            {
                if (i == startY && i == endY)
                    oss << lines[i].substr(startX, endX - startX);
                else if (i == startY)
                    oss << lines[i].substr(startX) << "\n";
                else if (i == endY)
                    oss << lines[i].substr(0, endX);
                else
                    oss << lines[i] << "\n";
            }
            SetClipboardText(oss.str().c_str());
        }
    }
    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_X))
    {
        if (selectionActive)
        {
            // Thực hiện CUT: copy rồi xóa vùng chọn
            int startY = selStartY, endY = selEndY, startX = selStartX, endX = selEndX;
            if (startY > endY || (startY == endY && startX > endX))
            {
                std::swap(startY, endY);
                std::swap(startX, endX);
            }
            std::ostringstream oss;
            for (int i = startY; i <= endY; i++)
            {
                if (i == startY && i == endY)
                    oss << lines[i].substr(startX, endX - startX);
                else if (i == startY)
                    oss << lines[i].substr(startX) << "\n";
                else if (i == endY)
                    oss << lines[i].substr(0, endX);
                else
                    oss << lines[i] << "\n";
            }
            SetClipboardText(oss.str().c_str());
            DeleteSelection();
        }
    }
    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_V))
    {
        SaveStateForUndo();
        if (selectionActive)
        {
            DeleteSelection();
            selectionActive = false;
        }
        const char *clipText = GetClipboardText();
        if (clipText != nullptr)
        {
            std::string pasteText = clipText;
            // Làm sạch: loại bỏ ký tự không mong muốn ở cuối mỗi dòng
            std::vector<std::string> pasteLines;
            std::istringstream iss(pasteText);
            std::string temp;
            while (std::getline(iss, temp))
            {
                // Nếu dòng kết thúc bằng '\r', loại bỏ nó
                if (!temp.empty() && temp.back() == '\r')
                    temp.pop_back();
                // Loại bỏ ký tự không in được và khoảng trắng thừa bên phải
                temp.erase(std::remove_if(temp.begin(), temp.end(), [](char c)
                                          { return !isprint(static_cast<unsigned char>(c)); }),
                           temp.end());
                while (!temp.empty() && isspace(temp.back()))
                    temp.pop_back();
                pasteLines.push_back(temp);
            }
            if (pasteLines.size() == 1)
            {
                lines[cursorY].insert(cursorX, pasteLines[0]);
                cursorX += pasteLines[0].size();
            }
            else
            {
                std::string leftPart = lines[cursorY].substr(0, cursorX);
                std::string rightPart = lines[cursorY].substr(cursorX);
                lines[cursorY] = leftPart + pasteLines[0];
                for (size_t i = 1; i < pasteLines.size(); i++)
                {
                    lines.insert(lines.begin() + cursorY + i, pasteLines[i]);
                }
                cursorY = cursorY + pasteLines.size() - 1;
                cursorX = pasteLines.back().size();
                lines[cursorY] += rightPart;
            }
        }
    }

    // ===== XỬ LÝ NHẬP KÝ TỰ THÔNG QUA BÀN PHÍM =====
    int key = GetCharPressed();
    while (key > 0)
    {
        if (key >= 32 && key <= 125)
        {
            SaveStateForUndo();
            if (selectionActive)
            {
                DeleteSelection();
                selectionActive = false;
            }
            lines[cursorY].insert(cursorX, 1, (char)key);
            cursorX++;
        }
        key = GetCharPressed();
    }

    // ===== XỬ LÝ DI CHUYỂN CON TRỎ =====
    if (IsKeyPressed(KEY_LEFT))
    {
        if (cursorX > 0)
            cursorX--;
        else if (cursorY > 0)
        {
            cursorY--;
            cursorX = (int)lines[cursorY].size();
        }
        selectionActive = false;
    }
    if (IsKeyPressed(KEY_RIGHT))
    {
        if (cursorX < (int)lines[cursorY].size())
            cursorX++;
        else if (cursorY < (int)lines.size() - 1)
        {
            cursorY++;
            cursorX = 0;
        }
        selectionActive = false;
    }
    if (IsKeyPressed(KEY_UP))
    {
        if (cursorY > 0)
        {
            cursorY--;
            cursorX = std::min(cursorX, (int)lines[cursorY].size());
        }
        selectionActive = false;
    }
    if (IsKeyPressed(KEY_DOWN))
    {
        if (cursorY < (int)lines.size() - 1)
        {
            cursorY++;
            cursorX = std::min(cursorX, (int)lines[cursorY].size());
        }
        selectionActive = false;
    }

    // Backspace xử lý
    if (IsKeyPressed(KEY_BACKSPACE))
    {
        SaveStateForUndo();
        if (cursorX > 0)
        {
            lines[cursorY].erase(cursorX - 1, 1);
            cursorX--;
        }
        else if (cursorY > 0)
        {
            int prevLen = (int)lines[cursorY - 1].size();
            lines[cursorY - 1] += lines[cursorY];
            lines.erase(lines.begin() + cursorY);
            cursorY--;
            cursorX = prevLen;
        }
        selectionActive = false;
    }

    // Xử lý Enter (xuống dòng)
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))
    {
        SaveStateForUndo();
        std::string newLine = lines[cursorY].substr(cursorX);
        lines[cursorY] = lines[cursorY].substr(0, cursorX);
        lines.insert(lines.begin() + cursorY + 1, newLine);
        cursorY++;
        cursorX = 0;
        selectionActive = false;
    }

    // ===== XỬ LÝ CHỌN VĂN BẢN BẰNG CHUỘT =====
    Vector2 mousePos = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, inputBox))
    {
        // Khi click chuột, bắt đầu chọn
        isSelecting = true;
        selectionActive = true;
        GetCursorFromMousePos(mousePos, selStartY, selStartX, inputBox);
        // Ban đầu vùng chọn bắt đầu và kết thúc giống nhau
        selEndX = selStartX;
        selEndY = selStartY;
        // Cập nhật con trỏ theo vị trí click
        cursorX = selEndX;
        cursorY = selEndY;
    }
    if (isSelecting && IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        // Khi kéo chuột, cập nhật vị trí kết thúc vùng chọn
        GetCursorFromMousePos(mousePos, selEndY, selEndX, inputBox);
        // Cập nhật con trỏ theo vị trí hiện tại
        cursorX = selEndX;
        cursorY = selEndY;
    }
    // Nếu thả chuột, kết thúc quá trình chọn
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        isSelecting = false;
    }

    // ===== XỬ LÝ SCROLL =====
    if (CheckCollisionPointRec(mousePos, inputBox))
    {
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f)
            scrollOffset -= wheel * lineHeight;
    }
    float totalHeight = lines.size() * lineHeight;
    float maxScroll = totalHeight - inputBox.height;
    if (maxScroll < 0)
        maxScroll = 0;
    if (scrollOffset < 0)
        scrollOffset = 0;
    if (scrollOffset > maxScroll)
        scrollOffset = maxScroll;

    // Vẽ nền input box
    DrawRectangleRec(inputBox, LIGHTGRAY);
    DrawRectangleLinesEx(inputBox, 2, BLACK);

    // Giới hạn vẽ trong input box
    BeginScissorMode((int)inputBox.x, (int)inputBox.y, (int)inputBox.width, (int)inputBox.height);

    // Vẽ vùng chọn nếu có (nên vẽ trước khi vẽ text để text hiển thị trên nền chọn)
    DrawSelectionHighlight(inputBox);

    // Vẽ từng dòng text
    float y = inputBox.y - scrollOffset;
    for (size_t i = 0; i < lines.size(); i++)
    {
        DrawText(lines[i].c_str(), inputBox.x + padding, y, fontSize, BLACK);
        y += lineHeight;
    }
    EndScissorMode();

    // Vẽ con trỏ (đường thẳng dọc màu đỏ)
    frameCounter++;
    if ((frameCounter / 20) % 2 == 0 && cursorX >= 0 && cursorY >= 0 && cursorY < (int)lines.size())
    {
        std::string curText = lines[cursorY].substr(0, cursorX);
        int cursorOffset = MeasureText(curText.c_str(), fontSize);
        float cursorXPos = inputBox.x + padding + cursorOffset;
        float cursorYPos = inputBox.y + cursorY * lineHeight - scrollOffset;
        if (!(cursorYPos < inputBox.y || cursorYPos > inputBox.y + inputBox.height))
            DrawLine(cursorXPos, cursorYPos, cursorXPos, cursorYPos + fontSize, RED);
    }
    return lines;
}