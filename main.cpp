#include "raylib.h"
#include <string>
#include <sstream>
#include <cctype>
#include <cmath>
#include <algorithm>
#include <stack>
#include <iomanip>

const int screenWidth = 400;
const int screenHeight = 540; 

struct Button {
    Rectangle rect;
    const char* label;
};

// --- Calculator logic ---
bool IsOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}

int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '^') return 3;
    return 0;
}

float EvaluateExpression(const std::string& expr) {
    std::stack<float> values;
    std::stack<char> ops;
    bool valuePushed = false;
    auto applyOperator = [](std::stack<float>& values, std::stack<char>& ops) {
        if (values.size() < 2 || ops.empty()) throw std::runtime_error("Invalid expression");
        float b = values.top(); values.pop();
        float a = values.top(); values.pop();
        char op = ops.top(); ops.pop();
        switch (op) {
            case '+': values.push(a + b); break;
            case '-': values.push(a - b); break;
            case '*': values.push(a * b); break;
            case '/': values.push(a / b); break;
            case '^': values.push(pow(a, b)); break;
        }
    };
    for (size_t i = 0; i < expr.size(); ++i) {
        char c = expr[i];
        if (isspace(c)) continue;
        if (isdigit(c) || c == '.') {
            std::string numStr;
            while (i < expr.size() && (isdigit(expr[i]) || expr[i] == '.')) {
                numStr += expr[i];
                ++i;
            }
            --i;
            values.push(std::stof(numStr));
            valuePushed = true;
        } else if (IsOperator(c)) {
            while (!ops.empty() && precedence(ops.top()) >= precedence(c)) {
                applyOperator(values, ops);
            }
            ops.push(c);
        }
    }
    while (!ops.empty()) {
        applyOperator(values, ops);
    }
    if (values.size() != 1 || !valuePushed) throw std::runtime_error("Invalid expression");
    return values.top();
}

bool CanAddDot(const std::string& expr) {
    int i = expr.size() - 1;
    while (i >= 0 && !IsOperator(expr[i])) {
        if (expr[i] == '.') return false;
        i--;
    }
    return true;
}

int main() {
    // Set window to borderless 
    SetConfigFlags(FLAG_WINDOW_UNDECORATED);
    InitWindow(screenWidth, screenHeight, "Calculator");
    Font customFont = LoadFont("Ubuntu-Bold.ttf");
    Image icon = LoadImage("calc.png");
    SetWindowIcon(icon);
    UnloadImage(icon);
    SetTargetFPS(30);
    Button buttons[] = {
        {{20, 170, 80, 60}, "x^y"}, {{110, 170, 80, 60}, "/"}, {{200, 170, 80, 60}, "*"}, {{290, 170, 80, 60}, "C"},
        {{20, 240, 80, 60}, "7"}, {{110, 240, 80, 60}, "8"}, {{200, 240, 80, 60}, "9"}, {{290, 240, 80, 60}, "-"},
        {{20, 310, 80, 60}, "4"}, {{110, 310, 80, 60}, "5"}, {{200, 310, 80, 60}, "6"}, {{290, 310, 80, 60}, "+"},
        {{20, 380, 80, 60}, "1"}, {{110, 380, 80, 60}, "2"}, {{200, 380, 80, 60}, "3"}, {{290, 380, 80, 130}, "="},
        {{20, 450, 170, 60}, "0"}, {{200, 450, 80, 60}, "."}
    };
    std::string displayText = "";
    std::string currentInput = "0";
    bool inputStarted = false;
    Vector2 mouse;
    bool showContextMenu = false;
    Rectangle contextMenu = { 300, 60, 80, 50 };
    while (!WindowShouldClose()) {
        mouse = GetMousePosition();
        Rectangle closeBtn = {screenWidth - 40, 0, 40, 30};
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, closeBtn)) {
            break;
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !currentInput.empty()) {
            currentInput.pop_back();
        }
        static double lastRepeatTime[10] = {0};
        static bool keyHeld[10] = {false};
        double now = GetTime();
        for (int i = 0; i <= 9; i++) {
            bool pressed = IsKeyPressed(KEY_ZERO + i) || IsKeyPressed(KEY_KP_0 + i);
            bool held = (IsKeyDown(KEY_ZERO + i) || IsKeyDown(KEY_KP_0 + i));
            if (pressed) {
                if (!inputStarted && displayText.empty() && (currentInput != "ERR" && currentInput != "Inf" && currentInput != "-")) {
                    currentInput = "";
                    inputStarted = true;
                }
                if (currentInput.length() < 16) {
                    if (currentInput == "0" || currentInput.empty()) currentInput = std::to_string(i);
                    else if (currentInput == "-") currentInput = "-" + std::to_string(i);
                    else if (currentInput == "-0") currentInput += std::to_string(i);
                    else if (currentInput.substr(0,2) == "-0" && currentInput.length() > 2) currentInput += std::to_string(i);
                    else if (currentInput == "0.") currentInput += std::to_string(i);
                    else if (currentInput == "-0.") currentInput += std::to_string(i);
                    else if (currentInput[0] == '-' && (currentInput.length() > 1 && currentInput[1] != '0')) currentInput += std::to_string(i);
                    else currentInput += std::to_string(i);
                }
                lastRepeatTime[i] = now;
                keyHeld[i] = true;
            } else if (held && keyHeld[i] && now - lastRepeatTime[i] > 0.35) {
                if (!inputStarted && displayText.empty() && (currentInput != "ERR" && currentInput != "Inf" && currentInput != "-")) {
                    currentInput = "";
                    inputStarted = true;
                }
                if (currentInput.length() < 16) {
                    if (currentInput == "0" || currentInput.empty()) currentInput = std::to_string(i);
                    else if (currentInput == "-") currentInput = "-" + std::to_string(i);
                    else if (currentInput == "-0") currentInput += std::to_string(i);
                    else if (currentInput.substr(0,2) == "-0" && currentInput.length() > 2) currentInput += std::to_string(i);
                    else if (currentInput == "0.") currentInput += std::to_string(i);
                    else if (currentInput == "-0.") currentInput += std::to_string(i);
                    else if (currentInput[0] == '-' && (currentInput.length() > 1 && currentInput[1] != '0')) currentInput += std::to_string(i);
                    else currentInput += std::to_string(i);
                }
                lastRepeatTime[i] = now - 0.25;
            } else if (!held) {
                keyHeld[i] = false;
            }
        }
        if ((IsKeyPressed(KEY_PERIOD) || IsKeyPressed(KEY_KP_DECIMAL)) && CanAddDot(currentInput) && currentInput.length() < 16) {
            if (currentInput.empty() || currentInput == "0") currentInput = "0.";
            else if (currentInput == "-") currentInput = "-0.";
            else currentInput += ".";
        }
        const struct { int key; char op; } keyOps[] = {
            { KEY_KP_ADD, '+' }, { KEY_MINUS, '-' }, { KEY_KP_SUBTRACT, '-' },
            { KEY_KP_MULTIPLY, '*' }, { KEY_SLASH, '/' }, { KEY_KP_DIVIDE, '/' }
        };
        for (auto ko : keyOps) {
            if (IsKeyPressed(ko.key)) {
                if (ko.op == '-' && (currentInput == "0" || currentInput.empty())) {
                    if (currentInput == "0" && displayText.empty()) {
                        displayText = "0-";
                        currentInput = "";
                        inputStarted = false;
                    } else {
                        currentInput = "-";
                    }
                } else if (!currentInput.empty() && currentInput != "ERR" && currentInput != "Inf" && currentInput != "-") {
                    displayText += currentInput + ko.op;
                    currentInput.clear();
                    inputStarted = false;
                }
            }
        }
        static bool dragging = false;
        static Vector2 dragOffset = {0, 0};
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, {0, 0, screenWidth, 40})) {
            dragging = true;
            dragOffset = mouse;
        }
        if (dragging) {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                Vector2 pos = GetWindowPosition();
                SetWindowPosition((int)(pos.x + (mouse.x - dragOffset.x)), (int)(pos.y + (mouse.y - dragOffset.y)));
            } else {
                dragging = false;
            }
        }
        if ((IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER))) {
            std::string expr = displayText + currentInput;
            if (expr.empty() || expr == "." || expr == "-") {
                displayText.clear();
                currentInput = "0";
            } else {
                if (!expr.empty() && expr[0] == '-') expr = "0" + expr;
                try {
                    float result = EvaluateExpression(expr);
                    displayText.clear();
                    std::ostringstream oss;
                    if (!std::isfinite(result)) {
                        currentInput = "Inf";
                    } else if (std::abs(result) >= 1e10f || (std::abs(result) > 0 && std::abs(result) < 1e-6f)) {
                        oss << std::setprecision(6) << std::scientific << result;
                        currentInput = oss.str();
                    } else {
                        oss << std::fixed << std::setprecision(3) << result;
                        std::string s = oss.str();
                        
                        size_t dot = s.find('.');
                        if (dot != std::string::npos) {
                            size_t last_nonzero = s.find_last_not_of('0');
                            if (last_nonzero != std::string::npos && last_nonzero > dot) {
                                s.erase(last_nonzero + 1);
                            } else {
                                s.erase(dot); 
                            }
                        }
                        currentInput = s;
                    }
                } catch (...) {
                    displayText.clear();
                    currentInput = "ERR";
                }
            }
            inputStarted = false;
        }
        if (IsKeyPressed(KEY_C) || IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_ESCAPE)) {
            displayText.clear();
            currentInput = "0";
            inputStarted = false;
        }
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            if (CheckCollisionPointRec(mouse, {20, 50, 350, 80})) {
                float menuWidth = contextMenu.width;
                float menuHeight = contextMenu.height;
                float x = mouse.x;
                float y = mouse.y;
                if (x + menuWidth > screenWidth) x = mouse.x - menuWidth;
                if (x < 0) x = 0;
                if (y + menuHeight > screenHeight) y = screenHeight - menuHeight;
                if (y < 0) y = 0;
                contextMenu.x = x;
                contextMenu.y = y;
                showContextMenu = true;
            } else {
                showContextMenu = false;
            }
        }
        if (showContextMenu && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mouse, {contextMenu.x, contextMenu.y, contextMenu.width, 25})) {
                showContextMenu = false;
            } else if (CheckCollisionPointRec(mouse, {contextMenu.x, contextMenu.y + 25, contextMenu.width, 25})) {
                showContextMenu = false;
            } else {
                showContextMenu = false;
            }
        }
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            for (auto& button : buttons) {
                if (CheckCollisionPointRec(mouse, button.rect)) {
                    std::string label = button.label;
                    if (label == "C") {
                        displayText.clear();
                        currentInput = "0";
                        inputStarted = false;
                    } else if (label == "=") {
                        std::string expr = displayText + currentInput;
                        if (expr.empty() || expr == "." || expr == "-") {
                            displayText.clear();
                            currentInput.clear();
                        } else {
                            if (!expr.empty() && expr[0] == '-') expr = "0" + expr;
                            try {
                                float result = EvaluateExpression(expr);
                                displayText.clear();
                                std::ostringstream oss;
                                if (!std::isfinite(result)) {
                                    currentInput = "Inf";
                                } else if (std::abs(result) >= 1e10f || (std::abs(result) > 0 && std::abs(result) < 1e-6f)) {
                                    oss << std::setprecision(6) << std::scientific << result;
                                    currentInput = oss.str();
                                } else {
                                    oss << std::fixed << std::setprecision(3) << result;
                                    std::string s = oss.str();
                                    // Remove trailing zeros and dot
                                    size_t dot = s.find('.');
                                    if (dot != std::string::npos) {
                                        size_t last_nonzero = s.find_last_not_of('0');
                                        if (last_nonzero != std::string::npos && last_nonzero > dot) {
                                            s.erase(last_nonzero + 1);
                                        } else {
                                            s.erase(dot); 
                                        }
                                    }
                                    currentInput = s;
                                }
                            } catch (...) {
                                displayText.clear();
                                currentInput = "ERR";
                            }
                        }
                        inputStarted = false;
                    } else if (label == "x^y") {
                        if (!currentInput.empty() && currentInput != "-") {
                            displayText += currentInput + '^';
                            currentInput.clear();
                            inputStarted = false;
                        }
                    } else if (label == ".") {
                        if (CanAddDot(currentInput) && currentInput.length() < 16) {
                            if (currentInput.empty() || currentInput == "0") currentInput = "0.";
                            else if (currentInput == "-") currentInput = "-0.";
                            else currentInput += ".";
                        }
                    } else if (std::isdigit(label[0]) && currentInput.length() < 16) {
                        if (!inputStarted && displayText.empty() && (currentInput != "ERR" && currentInput != "Inf" && currentInput != "-")) {
                            currentInput = "";
                            inputStarted = true;
                        }
                        if (currentInput == "0" || currentInput.empty()) currentInput = label;
                        else if (currentInput == "-") currentInput = "-" + label;
                        else if (currentInput == "-0") currentInput += label;
                        else if (currentInput.substr(0,2) == "-0" && currentInput.length() > 2) currentInput += label;
                        else if (currentInput == "0.") currentInput += label;
                        else if (currentInput == "-0.") currentInput += label;
                        else if (currentInput[0] == '-' && (currentInput.length() > 1 && currentInput[1] != '0')) currentInput += label;
                        else currentInput += label;
                    } else if (IsOperator(label[0])) {
                        if (label[0] == '-' && (currentInput == "0" || currentInput.empty())) {
                            if (currentInput == "0" && displayText.empty()) {
                                displayText = "0-";
                                currentInput = "";
                                inputStarted = false;
                            } else {
                                currentInput = "-";
                            }
                        } else if (!currentInput.empty() && currentInput != "ERR" && currentInput != "Inf" && currentInput != "-") {
                            displayText += currentInput + label[0];
                            currentInput.clear();
                            inputStarted = false;
                        }
                    }
                    break;
                }
            }
        }
        BeginDrawing();
        ClearBackground(BROWN);
        DrawRectangle(0, 0, screenWidth, 40, DARKBROWN);
        DrawText("Calculator", 20, 10, 20, RAYWHITE);
        Color closeColor = (CheckCollisionPointRec(mouse, closeBtn) ? RED : GRAY);
        DrawRectangleRec(closeBtn, closeColor);
        DrawText("X", closeBtn.x + 14, closeBtn.y + 5, 18, RAYWHITE);
        DrawRectangle(20, 50, 350, 80, DARKBROWN);
        DrawRectangleLines(20, 50, 350, 80, DARKBROWN);
        std::string exprShown = displayText.empty() ? "" : displayText;
        float exprFontSize = 20;
        Vector2 exprSize = MeasureTextEx(customFont, exprShown.c_str(), exprFontSize, 1);
        float exprX = 360 - exprSize.x - 10;
        if (exprX < 25) exprX = 25;
        DrawTextEx(customFont, exprShown.c_str(), { exprX, 65 }, exprFontSize, 1, RAYWHITE);
        std::string inputShown = (currentInput.empty() || currentInput == ".") ? "0" : currentInput;
        float inputFontSize = 32;
        Vector2 inputSize = MeasureTextEx(customFont, inputShown.c_str(), inputFontSize, 1);
        float inputX = 360 - inputSize.x - 10;
        if (inputX < 25) inputX = 25;
        DrawTextEx(customFont, inputShown.c_str(), { inputX, 98 }, inputFontSize, 1, RAYWHITE);
        for (auto& button : buttons) {
            Color fillColor = DARKBROWN;
            if (CheckCollisionPointRec(mouse, button.rect)) fillColor = BEIGE;
            DrawRectangleRounded(button.rect, 0.4f, 12, fillColor);
            Vector2 labelSize = MeasureTextEx(customFont, button.label, 20, 1);
            Vector2 labelPos = {
                button.rect.x + button.rect.width / 2 - labelSize.x / 2,
                button.rect.y + button.rect.height / 2 - labelSize.y / 2
            };
            DrawTextEx(customFont, button.label, labelPos, 20, 1, BLACK);
        }
        if (showContextMenu) {
            DrawRectangleRec(contextMenu, DARKGRAY);
            DrawText("Copy", contextMenu.x + 5, contextMenu.y + 5, 18, RAYWHITE);
            DrawText("Paste", contextMenu.x + 5, contextMenu.y + 30, 18, RAYWHITE);
        }
        EndDrawing();
    }
    UnloadFont(customFont);
    CloseWindow();
    return 0;
}
