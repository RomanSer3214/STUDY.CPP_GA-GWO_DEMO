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
        
        // Знаходимо реальні мінімум і максимум для відображення
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
        
        // Додаємо відступи
        float padding = (maxY - minY) * 0.1f;
        if (padding < 0.1f) padding = 1.0f;
        minY -= padding;
        maxY += padding;

        // Малюємо фон
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddRectFilled(canvasPos, ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y), IM_COL32(20, 20, 20, 255));

        // Малюємо сітку та осі ПЕРШИМИ
        DrawDynamicGrid(canvasPos, canvasSize, minY, maxY, drawList);
        DrawAxes(canvasPos, canvasSize, minY, maxY, drawList);

        // Малюємо функцію
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

        // Використовуємо searchMin/searchMax для сітки (як у мапуванні)
        float visibleRangeX = searchMax - searchMin;
        float visibleRangeY = maxY - minY;
        
        float xStep = CalculateAdaptiveStep(visibleRangeX);
        float yStep = CalculateAdaptiveStep(visibleRangeY);
        
        // Сітка
        DrawGridLines(canvasPos, canvasSize, minY, maxY, xStep, yStep, gridColor, drawList);
        DrawGridLines(canvasPos, canvasSize, minY, maxY, xStep/2.0f, yStep/2.0f, subGridColor, drawList);
        
        // Підписи - ВСІ значення сітки
        DrawAllGridLabels(canvasPos, canvasSize, minY, maxY, xStep, yStep, drawList, textColor);
    }

    void DrawAxes(const ImVec2& canvasPos, const ImVec2& canvasSize, float minY, float maxY, ImDrawList* drawList) {
        ImU32 axisColor = IM_COL32(255, 255, 255, 255);

        // Calculate where the Y-axis should be drawn (at x=0 if visible, otherwise at the left edge)
        float yAxisX = canvasPos.x;
        if (searchMin <= 0 && searchMax >= 0) {
            // X=0 is within our view - draw Y axis at x=0
            yAxisX = MapXToScreen(0.0f, canvasPos, canvasSize);
        }
        
        // Calculate where the X-axis should be drawn (at y=0 if visible, otherwise at appropriate edge)
        float xAxisY = canvasPos.y + canvasSize.y;
        if (minY <= 0 && maxY >= 0) {
            // Y=0 is within our view - draw X axis at y=0
            xAxisY = MapYToScreen(0.0f, canvasPos, canvasSize, minY, maxY);
        }
        
        // Draw Y axis (vertical line) - трохи коротша для кращого вигляду
        drawList->AddLine(
            ImVec2(yAxisX, canvasPos.y + 10),
            ImVec2(yAxisX, canvasPos.y + canvasSize.y - 10),
            axisColor, 2.0f
        );
        
        // Draw X axis (horizontal line) - трохи коротша для кращого вигляду
        drawList->AddLine(
            ImVec2(canvasPos.x + 10, xAxisY),
            ImVec2(canvasPos.x + canvasSize.x - 10, xAxisY),
            axisColor, 2.0f
        );
    }

    void DrawAllGridLabels(const ImVec2& canvasPos, const ImVec2& canvasSize, float minY, float maxY, float xStep, float yStep, ImDrawList* drawList, ImU32 textColor) {
        // Знаходимо позиції осей
        float xAxisY = canvasPos.y + canvasSize.y;
        float yAxisX = canvasPos.x;
        
        if (searchMin <= 0 && searchMax >= 0) {
            yAxisX = MapXToScreen(0.0f, canvasPos, canvasSize);
        }
        if (minY <= 0 && maxY >= 0) {
            xAxisY = MapYToScreen(0.0f, canvasPos, canvasSize, minY, maxY);
        }

        // Підписи для осі X - ВСІ значення
        float startX = std::ceil(searchMin / xStep) * xStep;
        float endX = std::floor(searchMax / xStep) * xStep;
        
        for (float x = startX; x <= endX; x += xStep) {
            if (x < searchMin || x > searchMax) continue;
            
            float screenX = MapXToScreen(x, canvasPos, canvasSize);
            char buffer[32];
            FormatNumber(buffer, sizeof(buffer), x);
            
            ImVec2 textSize = ImGui::CalcTextSize(buffer);
            
            // Підпис біля осі X (трохи нижче осі)
            float textY = xAxisY + 8;
            drawList->AddText(ImVec2(screenX - textSize.x / 2, textY), textColor, buffer);
        }

        // Підписи для осі Y - ВСІ значення
        float startY = std::ceil(minY / yStep) * yStep;
        float endY = std::floor(maxY / yStep) * yStep;
        
        for (float y = startY; y <= endY; y += yStep) {
            if (y < minY || y > maxY) continue;
            
            float screenY = MapYToScreen(y, canvasPos, canvasSize, minY, maxY);
            char buffer[32];
            FormatNumber(buffer, sizeof(buffer), y);
            
            ImVec2 textSize = ImGui::CalcTextSize(buffer);
            
            // Підпис біля осі Y (трохи ліворуч від осі)
            float textX = yAxisX - textSize.x - 8;
            drawList->AddText(ImVec2(textX, screenY - textSize.y / 2), textColor, buffer);
        }
    }

    void DrawGridLines(const ImVec2& canvasPos, const ImVec2& canvasSize, float minY, float maxY, 
                      float xStep, float yStep, ImU32 color, ImDrawList* drawList) {
        // Вертикальні лінії - ВСІ значення
        float startX = std::ceil(searchMin / xStep) * xStep;
        float endX = std::floor(searchMax / xStep) * xStep;
        
        for (float x = startX; x <= endX; x += xStep) {
            if (x < searchMin || x > searchMax) continue;
            
            float screenX = MapXToScreen(x, canvasPos, canvasSize);
            drawList->AddLine(ImVec2(screenX, canvasPos.y), ImVec2(screenX, canvasPos.y + canvasSize.y), color, 1.0f);
        }

        // Горизонтальні лінії - ВСІ значення
        float startY = std::ceil(minY / yStep) * yStep;
        float endY = std::floor(maxY / yStep) * yStep;
        
        for (float y = startY; y <= endY; y += yStep) {
            if (y < minY || y > maxY) continue;
            
            float screenY = MapYToScreen(y, canvasPos, canvasSize, minY, maxY);
            drawList->AddLine(ImVec2(canvasPos.x, screenY), ImVec2(canvasPos.x + canvasSize.x, screenY), color, 1.0f);
        }
    }

    float CalculateAdaptiveStep(float range) {
        if (range <= 0) return 1.0f;
        
        float logRange = log10f(range);
        float power = floorf(logRange);
        float step = powf(10.0f, power);
        
        // Зменшуємо крок для більш детальної сітки
        if (range / step > 20.0f) step *= 5.0f;
        else if (range / step > 10.0f) step *= 2.0f;
        
        // Додатково: якщо крок занадто великий, ділимо його
        if (range / step < 3.0f) {
            step /= 2.0f;
        }
        if (range / step < 3.0f) {
            step /= 2.5f;
        }
        
        return step;
    }

    void FormatNumber(char* buffer, size_t size, float value) {
        snprintf(buffer, size, "%g", value);
    }
};
