#include <vector>
#include <functional>
#include <cmath>
#include <cstdio>
#include "imgui.h"

class FunctionDrawer {
private:
    float searchMin, searchMax;
    std::function<float(float)> function;
    std::vector<float> functionPoints;
    std::vector<float> xValues;
    int resolution;

public:
    FunctionDrawer() : resolution(500) {}

    void Initialize(float min, float max, std::function<float(float)> func) {
        searchMin = min;
        searchMax = max;
        function = func;
        PrecomputeFunction();
    }

    void PrecomputeFunction() {
        functionPoints.clear();
        xValues.clear();
        functionPoints.resize(resolution);
        xValues.resize(resolution);

        for (int i = 0; i < resolution; ++i) {
            float x = searchMin + (searchMax - searchMin) * i / (resolution - 1);
            xValues[i] = x;
            functionPoints[i] = function(x);
        }
    }

    float MapXToScreen(float x, const ImVec2& canvasPos, const ImVec2& canvasSize) {
        return canvasPos.x + (x - searchMin) / (searchMax - searchMin) * canvasSize.x;
    }

    float MapYToScreen(float y, const ImVec2& canvasPos, const ImVec2& canvasSize, float minY, float maxY) {
        return canvasPos.y + canvasSize.y - (y - minY) / (maxY - minY) * canvasSize.y;
    }

    void DrawFunction() {
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();
        
        if (canvasSize.x < 50.0f || canvasSize.y < 50.0f) {
            canvasSize = ImVec2(800, 600);
        }
        
        // мінімум і максимум для відображення
        float minY = functionPoints[0];
        float maxY = functionPoints[0];
        for (int i = 1; i < resolution; ++i) {
            if (functionPoints[i] < minY) minY = functionPoints[i];
            if (functionPoints[i] > maxY) maxY = functionPoints[i];
        }
        
        // Обробка особливих випадків
        if (fabsf(maxY - minY) < 0.0001f) {
            minY -= 1.0f;
            maxY += 1.0f;
        }
        
        float padding = (maxY - minY) * 0.1f;
        if (padding < 0.1f) padding = 1.0f;
        minY -= padding;
        maxY += padding;

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(20, 20, 20, 255));

        DrawDynamicGrid(canvasPos, canvasSize, minY, maxY, drawList);
        DrawAxes(canvasPos, canvasSize, minY, maxY, drawList);

        ImU32 functionColor = IM_COL32(0, 255, 255, 255);
        
        for (int i = 0; i < resolution - 1; ++i) {
            float screenX1 = MapXToScreen(xValues[i], canvasPos, canvasSize);
            float screenY1 = MapYToScreen(functionPoints[i], canvasPos, canvasSize, minY, maxY);
            float screenX2 = MapXToScreen(xValues[i + 1], canvasPos, canvasSize);
            float screenY2 = MapYToScreen(functionPoints[i + 1], canvasPos, canvasSize, minY, maxY);
            
            // Перевірка на коректність значень
            if (std::isfinite(screenY1) && std::isfinite(screenY2) && 
                screenY1 >= canvasPos.y && screenY1 <= canvasPos.y + canvasSize.y &&
                screenY2 >= canvasPos.y && screenY2 <= canvasPos.y + canvasSize.y) {
                drawList->AddLine(ImVec2(screenX1, screenY1), ImVec2(screenX2, screenY2), functionColor, 2.0f);
            }
        }
    }

    void DrawSolutions(const std::vector<float>& solutions) {
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        ImVec2 canvasSize = ImGui::GetContentRegionAvail();
        
        if (canvasSize.x < 50.0f || canvasSize.y < 50.0f) return;
        
        float minY = functionPoints[0];
        float maxY = functionPoints[0];
        for (int i = 1; i < resolution; ++i) {
            if (functionPoints[i] < minY) minY = functionPoints[i];
            if (functionPoints[i] > maxY) maxY = functionPoints[i];
        }
        
        if (fabsf(maxY - minY) < 0.0001f) {
            minY -= 1.0f;
            maxY += 1.0f;
        }
        
        float padding = (maxY - minY) * 0.1f;
        if (padding < 0.1f) padding = 1.0f;
        minY -= padding;
        maxY += padding;

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        for (size_t i = 0; i < solutions.size(); ++i) {
            float x = solutions[i];
            float y = function(x);
            
            float screenX = MapXToScreen(x, canvasPos, canvasSize);
            float screenY = MapYToScreen(y, canvasPos, canvasSize, minY, maxY);

            if (screenX >= canvasPos.x && screenX <= canvasPos.x + canvasSize.x &&
                screenY >= canvasPos.y && screenY <= canvasPos.y + canvasSize.y) {
                
                ImU32 color = (i == 0) ? IM_COL32(255, 0, 0, 255) : IM_COL32(255, 255, 0, 255);
                drawList->AddCircleFilled(ImVec2(screenX, screenY), 6.0f, color);
                drawList->AddCircle(ImVec2(screenX, screenY), 6.0f, IM_COL32(255, 255, 255, 255), 0, 2.0f);
            }
        }
    }

private:
    void DrawDynamicGrid(const ImVec2& canvasPos, const ImVec2& canvasSize, float minY, float maxY, ImDrawList* drawList) {
        ImU32 gridColor = IM_COL32(100, 100, 100, 100);
        ImU32 subGridColor = IM_COL32(70, 70, 70, 60);
        ImU32 textColor = IM_COL32(200, 200, 200, 255);
        
        float xRange = searchMax - searchMin;
        float yRange = maxY - minY;
        
        float xStep = CalculateAdaptiveStep(xRange, canvasSize.x);
        float yStep = CalculateAdaptiveStep(yRange, canvasSize.y);
        
        DrawGridLines(canvasPos, canvasSize, minY, maxY, xStep, yStep, gridColor, drawList);
        DrawGridLines(canvasPos, canvasSize, minY, maxY, xStep * 0.5f, yStep * 0.5f, subGridColor, drawList);
        
        // Адаптивні підписи
        DrawAdaptiveGridLabels(canvasPos, canvasSize, minY, maxY, xStep, yStep, drawList, textColor);
    }

    void DrawAxes(const ImVec2& canvasPos, const ImVec2& canvasSize, float minY, float maxY, ImDrawList* drawList) {
        ImU32 axisColor = IM_COL32(255, 255, 255, 255);

        float yAxisX = canvasPos.x;
        if (searchMin <= 0 && searchMax >= 0) {
            yAxisX = MapXToScreen(0.0f, canvasPos, canvasSize);
        }
        
        float xAxisY = canvasPos.y + canvasSize.y;
        if (minY <= 0 && maxY >= 0) {
            xAxisY = MapYToScreen(0.0f, canvasPos, canvasSize, minY, maxY);
        }
        
        drawList->AddLine(
            ImVec2(yAxisX, canvasPos.y + 10),
            ImVec2(yAxisX, canvasPos.y + canvasSize.y - 10),
            axisColor, 2.0f
        );
        
        drawList->AddLine(
            ImVec2(canvasPos.x + 10, xAxisY),
            ImVec2(canvasPos.x + canvasSize.x - 10, xAxisY),
            axisColor, 2.0f
        );
    }

    void DrawAdaptiveGridLabels(const ImVec2& canvasPos, const ImVec2& canvasSize, float minY, float maxY, float xStep, float yStep, ImDrawList* drawList, ImU32 textColor) {
        float xAxisY = canvasPos.y + canvasSize.y;
        float yAxisX = canvasPos.x;
        
        if (searchMin <= 0 && searchMax >= 0) {
            yAxisX = MapXToScreen(0.0f, canvasPos, canvasSize);
        }
        if (minY <= 0 && maxY >= 0) {
            xAxisY = MapYToScreen(0.0f, canvasPos, canvasSize, minY, maxY);
        }

        // Мінімальна відстань між підписами в пікселях
        const float minLabelSpacingX = 60.0f;
        const float minLabelSpacingY = 30.0f;

        float startX = std::ceil(searchMin / xStep) * xStep;
        float endX = std::floor(searchMax / xStep) * xStep;
        
        float lastLabelX = -minLabelSpacingX; // Для відстеження останнього намальованого підпису
        
        for (float x = startX; x <= endX; x += xStep) {
            if (x < searchMin || x > searchMax) continue;
            
            float screenX = MapXToScreen(x, canvasPos, canvasSize);
            
            if (screenX - lastLabelX < minLabelSpacingX) continue;
            
            char buffer[32];
            FormatNumber(buffer, sizeof(buffer), x);
            
            ImVec2 textSize = ImGui::CalcTextSize(buffer);
            
            if (screenX - textSize.x/2 >= canvasPos.x && screenX + textSize.x/2 <= canvasPos.x + canvasSize.x) {
                float textY = xAxisY + 8;
                drawList->AddText(ImVec2(screenX - textSize.x / 2, textY), textColor, buffer);
                lastLabelX = screenX;
            }
        }

        float startY = std::ceil(minY / yStep) * yStep;
        float endY = std::floor(maxY / yStep) * yStep;
        
        float lastLabelY = canvasPos.y + canvasSize.y + minLabelSpacingY; // Для відстеження останнього намальованого підпису
        
        for (float y = startY; y <= endY; y += yStep) {
            if (y < minY || y > maxY) continue;
            
            float screenY = MapYToScreen(y, canvasPos, canvasSize, minY, maxY);
            
            // Перевірка відстані від попереднього підпису
            if (fabsf(screenY - lastLabelY) < minLabelSpacingY) continue;
            
            char buffer[32];
            FormatNumber(buffer, sizeof(buffer), y);
            
            ImVec2 textSize = ImGui::CalcTextSize(buffer);
            
            if (screenY - textSize.y/2 >= canvasPos.y && screenY + textSize.y/2 <= canvasPos.y + canvasSize.y) {
                float textX = yAxisX - textSize.x - 8;
                drawList->AddText(ImVec2(textX, screenY - textSize.y / 2), textColor, buffer);
                lastLabelY = screenY;
            }
        }
    }

    void DrawGridLines(const ImVec2& canvasPos, const ImVec2& canvasSize, float minY, float maxY, 
                      float xStep, float yStep, ImU32 color, ImDrawList* drawList) {
        // Вертикальні лінії
        float startX = std::ceil(searchMin / xStep) * xStep;
        float endX = std::floor(searchMax / xStep) * xStep;
        
        for (float x = startX; x <= endX; x += xStep) {
            if (x < searchMin || x > searchMax) continue;
            
            float screenX = MapXToScreen(x, canvasPos, canvasSize);
            drawList->AddLine(ImVec2(screenX, canvasPos.y), ImVec2(screenX, canvasPos.y + canvasSize.y), color, 1.0f);
        }

        // Горизонтальні лінії
        float startY = std::ceil(minY / yStep) * yStep;
        float endY = std::floor(maxY / yStep) * yStep;
        
        for (float y = startY; y <= endY; y += yStep) {
            if (y < minY || y > maxY) continue;
            
            float screenY = MapYToScreen(y, canvasPos, canvasSize, minY, maxY);
            drawList->AddLine(ImVec2(canvasPos.x, screenY), ImVec2(canvasPos.x + canvasSize.x, screenY), color, 1.0f);
        }
    }

    float CalculateAdaptiveStep(float range, float canvasSize) {
        float rawStep = range / (canvasSize / 80.0f); // Приблизно 80 пікселів між лініями
        
        float magnitude = powf(10.0f, floorf(log10f(rawStep)));
        float normalized = rawStep / magnitude;
        
        float step;
        if (normalized < 1.5f) {
            step = 1.0f;
        } 
        else if (normalized < 3.0f) {
            step = 2.0f;
        } 
        else if (normalized < 7.0f) {
            step = 5.0f;
        } 
        else {
            step = 10.0f;
        }
        
        return step * magnitude;
    }

    void FormatNumber(char* buffer, size_t size, float value) {
        if (value == 0.0f) {
            snprintf(buffer, size, "0");
            return;
        }

        snprintf(buffer, size, "%g", value);
    }
};
