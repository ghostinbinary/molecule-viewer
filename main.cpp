#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

using namespace std;

const float PI = 3.14159265f;

// Настройки
const float BOND_THICKNESS = 0.09f;     // радиус цилиндра связи
const float BOND_TOLERANCE = 1.25f;     // кандидат в связи, если расстояние < tolerance * (r1 + r2)
const float BOND_RELATIVE = 1.30f;      // ИЗМЕНЕНО: было 1.15f. Кратные связи короче, их соседние C-H нельзя отбрасывать
const float MOUSE_SENSITIVITY = 0.5f;   // скорость вращения при перетаскивании мышью
const float MIN_ZOOM = 0.3f;            // минимальный масштаб
const float MAX_ZOOM = 4.0f;            // максимальный масштаб
const int SPHERE_STEPS = 20;            // гладкость шара
const int BOND_STEPS = 12;              // гладкость цилиндра связи

// Таблица элементов
struct ElementInfo {
    const char* symbol;
    float bondRadius;    // нужен для поиска связей (ковалентный радиус, Å)
    float sphereRadius;  // размер шара на экране
    float r, g, b;       // цвет
};

const ElementInfo elements[] = {
    // символ  связь  шар     R      G      B
    { "H",     0.31f, 0.25f,  0.95f, 0.95f, 0.95f },
    { "He",    0.28f, 0.25f,  0.85f, 1.00f, 1.00f },
    { "Li",    1.28f, 0.55f,  0.80f, 0.50f, 1.00f },
    { "Be",    0.96f, 0.45f,  0.76f, 1.00f, 0.00f },
    { "B",     0.84f, 0.40f,  1.00f, 0.71f, 0.71f },
    { "C",     0.76f, 0.42f,  0.25f, 0.28f, 0.33f },
    { "N",     0.71f, 0.40f,  0.20f, 0.35f, 0.95f },
    { "O",     0.66f, 0.38f,  0.95f, 0.12f, 0.12f },
    { "F",     0.57f, 0.36f,  0.56f, 0.88f, 0.31f },
    { "Ne",    0.58f, 0.34f,  0.70f, 0.89f, 0.96f },
    { "Na",    1.66f, 0.55f,  0.67f, 0.36f, 0.95f },   // ИЗМЕНЕНО: реальный радиус 1.66 (было 0.75)
    { "Mg",    1.41f, 0.52f,  0.54f, 1.00f, 0.00f },
    { "Al",    1.21f, 0.50f,  0.75f, 0.65f, 0.65f },
    { "Si",    1.11f, 0.48f,  0.94f, 0.78f, 0.63f },
    { "P",     1.07f, 0.50f,  1.00f, 0.55f, 0.10f },
    { "S",     1.05f, 0.50f,  1.00f, 0.85f, 0.10f },
    { "Cl",    1.02f, 0.48f,  0.12f, 0.94f, 0.12f },
    { "Ar",    1.06f, 0.49f,  0.50f, 0.82f, 0.89f },
    { "K",     2.03f, 0.65f,  0.56f, 0.25f, 0.83f },
    { "Ca",    1.76f, 0.58f,  0.24f, 1.00f, 0.00f },
    { "Sc",    1.70f, 0.56f,  0.90f, 0.90f, 0.90f },
    { "Ti",    1.60f, 0.52f,  0.75f, 0.76f, 0.78f },
    { "V",     1.53f, 0.52f,  0.65f, 0.65f, 0.67f },
    { "Cr",    1.39f, 0.50f,  0.54f, 0.60f, 0.78f },
    { "Mn",    1.39f, 0.50f,  0.61f, 0.48f, 0.78f },
    { "Fe",    1.32f, 0.50f,  0.88f, 0.40f, 0.20f },
    { "Co",    1.26f, 0.50f,  0.94f, 0.56f, 0.63f },
    { "Ni",    1.24f, 0.50f,  0.31f, 0.82f, 0.31f },
    { "Cu",    1.32f, 0.50f,  0.78f, 0.50f, 0.20f },
    { "Zn",    1.22f, 0.50f,  0.49f, 0.50f, 0.69f },
    { "Ga",    1.22f, 0.50f,  0.76f, 0.56f, 0.56f },
    { "Ge",    1.20f, 0.50f,  0.40f, 0.56f, 0.56f },
    { "As",    1.19f, 0.50f,  0.74f, 0.50f, 0.89f },
    { "Se",    1.20f, 0.50f,  1.00f, 0.63f, 0.00f },
    { "Br",    1.20f, 0.52f,  0.65f, 0.16f, 0.16f },
    { "Kr",    1.16f, 0.52f,  0.36f, 0.72f, 0.82f },
    { "Rb",    2.20f, 0.68f,  0.44f, 0.18f, 0.69f },
    { "Sr",    1.95f, 0.62f,  0.00f, 1.00f, 0.00f },
    { "Y",     1.90f, 0.60f,  0.58f, 1.00f, 1.00f },
    { "Zr",    1.75f, 0.58f,  0.58f, 0.88f, 0.88f },
    { "Mo",    1.54f, 0.54f,  0.33f, 0.71f, 0.71f },
    { "Pd",    1.39f, 0.54f,  0.00f, 0.41f, 0.52f },
    { "Ag",    1.45f, 0.54f,  0.75f, 0.75f, 0.75f },
    { "Cd",    1.44f, 0.54f,  1.00f, 0.85f, 0.56f },
    { "In",    1.42f, 0.54f,  0.65f, 0.46f, 0.45f },
    { "Sn",    1.39f, 0.52f,  0.40f, 0.50f, 0.50f },
    { "Sb",    1.39f, 0.52f,  0.62f, 0.39f, 0.71f },
    { "Te",    1.38f, 0.52f,  0.83f, 0.48f, 0.00f },
    { "I",     1.39f, 0.56f,  0.58f, 0.00f, 0.58f },
    { "Xe",    1.40f, 0.56f,  0.26f, 0.62f, 0.69f },
    { "Cs",    2.44f, 0.70f,  0.34f, 0.09f, 0.56f },
    { "Ba",    2.15f, 0.65f,  0.00f, 0.79f, 0.00f },
    { "W",     1.62f, 0.54f,  0.13f, 0.58f, 0.84f },
    { "Pt",    1.36f, 0.54f,  0.82f, 0.82f, 0.88f },
    { "Au",    1.36f, 0.54f,  1.00f, 0.82f, 0.14f },
    { "Hg",    1.32f, 0.54f,  0.72f, 0.72f, 0.82f },
    { "Tl",    1.45f, 0.55f,  0.65f, 0.33f, 0.30f },
    { "Pb",    1.46f, 0.55f,  0.34f, 0.35f, 0.38f },
    { "Bi",    1.48f, 0.55f,  0.62f, 0.31f, 0.71f },
    { "U",     1.96f, 0.62f,  0.00f, 0.56f, 1.00f },
};

// Значения для всех элементов, которых нет в таблице.
const ElementInfo defaultElement = { "?", 0.75f, 0.42f, 0.65f, 0.55f, 0.75f };

// Ищет элемент в таблице по символу; если не нашли, возвращает defaultElement.
const ElementInfo& getElement(const string& symbol) {
    for (const ElementInfo& element : elements)
        if (symbol == element.symbol) return element;
    return defaultElement;
}

// Файлы молекул:
const char* moleculeFiles[] = {
    "water.xyz", "methane.xyz", "benzene.xyz",
    "caffeine.xyz", "ammonia.xyz", "bromomethane.xyz"
};
// Количество файлов в списке (ИЗМЕНЕНО: считается автоматически по размеру массива)
const int MOLECULE_COUNT = sizeof(moleculeFiles) / sizeof(moleculeFiles[0]);

// Данные молекулы
struct Atom {
    string element;   // символ элемента
    float x, y, z;    // координаты
};

vector<Atom> atoms;              // все атомы
vector<pair<int, int>> bonds;    // связи: пары номеров атомов

vector<string> usedElements;     // элементы, которые есть в текущем файле (для легенды)
set<string> hiddenElements;      // элементы, скрытые кнопкой-глазом

bool isHidden(const string& symbol) {
    return hiddenElements.count(symbol) > 0;
}

// Глобальные переменные для управления мышью
bool isDragging = false;       // зажата ли левая кнопка мыши
double lastMouseX = 0.0;       // прошлая позиция мыши
double lastMouseY = 0.0;
float rotX = 20.0f;            // углы поворота (в градусах)
float rotY = 25.0f;
float zoomLevel = 1.0f;        // масштаб (колесо мыши)
float cameraDistance = 8.0f;   // расстояние от камеры до молекулы

// ДОБАВЛЕНО: одинаковые щелочные и щелочноземельные металлы между собой не связываем (Na-Na в NaCl).
bool isMetal(const string& s) {
    return s == "Li" || s == "Na" || s == "K" || s == "Rb" || s == "Cs" ||
        s == "Be" || s == "Mg" || s == "Ca" || s == "Sr" || s == "Ba";
}

// Чтение XYZ: первая строка - число атомов, вторая - комментарий, затем элемент и x y z.
bool loadXYZ(const string& filename, GLFWwindow* window) {
    ifstream file(filename);

    // Если файл не нашёлся рядом с программой, ищем в папке проекта.
    if (!file.is_open()) {
        file.clear();
        file.open("C:/first_project/" + filename);
    }
    if (!file.is_open()) {
        cout << "File not found: " << filename << endl;
        return false;
    }

    int count;
    if (!(file >> count) || count < 1 || count > 1000) {
        cout << "Cannot read " << filename << endl;
        return false;
    }
    string comment;
    getline(file, comment); // Дочитываем первую строку.
    getline(file, comment); // Пропускаем строку-комментарий XYZ.

    // Сначала читаем во временный список, чтобы при ошибке не испортить текущую молекулу.
    vector<Atom> loaded;
    for (int i = 0; i < count; ++i) {
        Atom a;
        if (!(file >> a.element >> a.x >> a.y >> a.z)) {
            cout << "Invalid XYZ coordinates in " << filename << endl;
            return false;
        }
        loaded.push_back(a);
    }

    atoms = loaded;

    // Список элементов файла для легенды (без повторов, в порядке появления).
    // Скрытые элементы сбрасываем: новый файл показывается целиком.
    usedElements.clear();
    hiddenElements.clear();
    for (const Atom& a : atoms) {
        bool found = false;
        for (const string& s : usedElements)
            if (s == a.element) { found = true; break; }
        if (!found) usedElements.push_back(a.element);
    }

    // Переносим центр молекулы в точку (0, 0, 0).
    float cx = 0, cy = 0, cz = 0;
    for (const Atom& a : atoms) { cx += a.x; cy += a.y; cz += a.z; }
    cx /= count; cy /= count; cz /= count;

    // Заодно находим радиус молекулы (самый далёкий атом от центра).
    float size = 0;
    for (Atom& a : atoms) {
        a.x -= cx; a.y -= cy; a.z -= cz;
        float d = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
        if (d > size) size = d;
    }
    // Чем больше молекула, тем дальше камера.
    cameraDistance = (size + 1.0f) * 3.0f;
    zoomLevel = 1.0f;

    // Автоматически находим связи по расстоянию и ковалентным радиусам.
    // Шаг 1: собираем всех кандидатов и запоминаем лучшую связь каждого атома.
    // ratio = расстояние / (r1 + r2); чем меньше ratio, тем "правильнее" связь.
    struct Candidate { int i, j; float ratio; };
    vector<Candidate> candidates;
    vector<float> bestRatio(count, 1000.0f);

    for (int i = 0; i < count; ++i) {
        for (int j = i + 1; j < count; ++j) {
            // ДОБАВЛЕНО: между одинаковыми щелочными/щелочноземельными металлами связей нет
            if (atoms[i].element == atoms[j].element && isMetal(atoms[i].element)) continue;
            float dx = atoms[i].x - atoms[j].x;
            float dy = atoms[i].y - atoms[j].y;
            float dz = atoms[i].z - atoms[j].z;
            float distance = sqrt(dx * dx + dy * dy + dz * dz);
            float sumR = getElement(atoms[i].element).bondRadius +
                getElement(atoms[j].element).bondRadius;
            float ratio = distance / sumR;
            if (distance > 0.4f && ratio < BOND_TOLERANCE) {
                candidates.push_back({ i, j, ratio });
                if (ratio < bestRatio[i]) bestRatio[i] = ratio;
                if (ratio < bestRatio[j]) bestRatio[j] = ratio;
            }
        }
    }

    // Шаг 2: оставляем связь, только если она не сильно хуже лучшей связи обоих атомов.
    bonds.clear();
    for (const Candidate& c : candidates) {
        if (c.ratio <= bestRatio[c.i] * BOND_RELATIVE &&
            c.ratio <= bestRatio[c.j] * BOND_RELATIVE)
            bonds.push_back({ c.i, c.j });
    }

    // Заголовок окна: имя файла, число атомов и связей.
    string title = filename + " - " + to_string(count) +
        " atoms, " + to_string(bonds.size()) + " bonds";
    glfwSetWindowTitle(window, title.c_str());
    return true;
}

// Шар, нарисованный несколькими полосками по широте.
void drawSphere(float radius) {
    for (int i = 0; i < SPHERE_STEPS; ++i) {
        float lat1 = -PI / 2 + PI * i / SPHERE_STEPS;        // нижняя граница полоски
        float lat2 = -PI / 2 + PI * (i + 1) / SPHERE_STEPS;  // верхняя граница полоски
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= SPHERE_STEPS; ++j) {
            float lon = 2 * PI * j / SPHERE_STEPS;           // долгота
            float x = cos(lon), z = sin(lon);
            // Нормаль направлена от центра шара наружу (нужна для освещения).
            glNormal3f(cos(lat1) * x, sin(lat1), cos(lat1) * z);
            glVertex3f(radius * cos(lat1) * x, radius * sin(lat1), radius * cos(lat1) * z);
            glNormal3f(cos(lat2) * x, sin(lat2), cos(lat2) * z);
            glVertex3f(radius * cos(lat2) * x, radius * sin(lat2), radius * cos(lat2) * z);
        }
        glEnd();
    }
}

// Тонкий цилиндр между двумя атомами (химическая связь).
void drawBond(const Atom& a, const Atom& b) {
    // Вектор от атома a к атому b и его длина.
    float dx = b.x - a.x, dy = b.y - a.y, dz = b.z - a.z;
    float length = sqrt(dx * dx + dy * dy + dz * dz);
    if (length < 0.001f) return;

    glPushMatrix();                    // запоминаем систему координат
    glTranslatef(a.x, a.y, a.z);       // переносим начало координат в атом a
    // Цилиндр строится вдоль оси Z, поэтому поворачиваем его в сторону атома b.
    float angle = acos(dz / length) * 180.0f / PI;
    if (fabs(dx) + fabs(dy) > 0.0001f)
        glRotatef(angle, -dy, dx, 0);
    else if (dz < 0) glRotatef(180, 1, 0, 0);   // связь строго вдоль Z, но вниз

    glColor3f(0.70f, 0.70f, 0.73f);    // светло-серый цвет связи
    glBegin(GL_QUAD_STRIP);
    for (int k = 0; k <= BOND_STEPS; ++k) {
        float t = 2 * PI * k / BOND_STEPS, x = cos(t), y = sin(t);
        glNormal3f(x, y, 0);
        glVertex3f(BOND_THICKNESS * x, BOND_THICKNESS * y, 0);        // нижнее кольцо
        glVertex3f(BOND_THICKNESS * x, BOND_THICKNESS * y, length);   // верхнее кольцо
    }
    glEnd();
    glPopMatrix();                     // возвращаем систему координат
}

// Кнопка-глаз: открытый глаз = элемент виден, перечёркнутый = скрыт.
// Возвращает true в кадре, когда на неё нажали.
bool eyeButton(const char* id, bool visible) {
    ImVec2 size(28.0f, ImGui::GetTextLineHeight());
    ImVec2 p = ImGui::GetCursorScreenPos();
    bool clicked = ImGui::InvisibleButton(id, size);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImU32 col = visible ? IM_COL32(235, 235, 235, 255) : IM_COL32(120, 120, 120, 255);
    float cx = p.x + size.x * 0.5f, cy = p.y + size.y * 0.5f;
    float w = size.x * 0.5f, h = size.y * 0.5f;
    ImVec2 left(cx - w + 2, cy), right(cx + w - 2, cy);

    // Контур глаза: две дуги (верхняя и нижняя) и зрачок.
    dl->AddBezierQuadratic(left, ImVec2(cx, cy - h * 1.6f), right, col, 1.5f);
    dl->AddBezierQuadratic(left, ImVec2(cx, cy + h * 1.6f), right, col, 1.5f);
    dl->AddCircleFilled(ImVec2(cx, cy), h * 0.45f, col);
    if (!visible)
        dl->AddLine(ImVec2(cx - w + 4, cy + h), ImVec2(cx + w - 4, cy - h), col, 2.0f);
    return clicked;
}

// Легенда в правом верхнем углу: цветной кружок, символ элемента, глаз.
void drawLegend() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 10.0f, 10.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.65f);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing;

    ImGui::Begin("Elements", nullptr, flags);
    ImGui::TextDisabled("Elements");
    ImGui::Separator();

    for (int i = 0; i < (int)usedElements.size(); ++i) {
        const string& symbol = usedElements[i];
        const ElementInfo& info = getElement(symbol);
        bool visible = !isHidden(symbol);
        ImGui::PushID(i);

        // Цветной кружок (у скрытого элемента - полупрозрачный).
        float h = ImGui::GetTextLineHeight();
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImU32 fill = IM_COL32((int)(info.r * 255), (int)(info.g * 255), (int)(info.b * 255), visible ? 255 : 70);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddCircleFilled(ImVec2(p.x + h * 0.5f, p.y + h * 0.5f), h * 0.5f, fill);
        dl->AddCircle(ImVec2(p.x + h * 0.5f, p.y + h * 0.5f), h * 0.5f, IM_COL32(200, 200, 200, visible ? 200 : 70));
        ImGui::Dummy(ImVec2(h, h));

        // Символ элемента.
        ImGui::SameLine();
        if (visible) ImGui::Text("%s", symbol.c_str());
        else         ImGui::TextDisabled("%s", symbol.c_str());

        // Глаз справа.
        ImGui::SameLine(75.0f);
        if (eyeButton("##eye", visible)) {
            if (visible) hiddenElements.insert(symbol);
            else         hiddenElements.erase(symbol);
        }
        ImGui::PopID();
    }
    ImGui::End();
}

// Обработчики ввода

// Нажатие и отпускание левой кнопки мыши.
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            // Клик по интерфейсу (легенде) не должен вращать молекулу.
            if (ImGui::GetIO().WantCaptureMouse) return;
            isDragging = true;
            // Запоминаем, где началось перетаскивание.
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
        }
        else if (action == GLFW_RELEASE) {
            isDragging = false;
        }
    }
}

// Движение мыши: если кнопка зажата, поворачиваем молекулу.
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (isDragging) {
        // Насколько сдвинулась мышь с прошлого раза.
        double deltaX = xpos - lastMouseX;
        double deltaY = ypos - lastMouseY;

        rotY += (float)deltaX * MOUSE_SENSITIVITY;   // влево-вправо: вокруг оси Y
        rotX += (float)deltaY * MOUSE_SENSITIVITY;   // вверх-вниз: вокруг оси X

        lastMouseX = xpos;
        lastMouseY = ypos;
    }
}

// Колесо мыши: приближение и отдаление.
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (ImGui::GetIO().WantCaptureMouse) return;
    zoomLevel *= yoffset > 0 ? 1.1f : 0.9f;
    if (zoomLevel < MIN_ZOOM) zoomLevel = MIN_ZOOM;
    if (zoomLevel > MAX_ZOOM) zoomLevel = MAX_ZOOM;
}

// Клавиатура: ESC - выход, цифры - загрузка молекулы из списка.
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);

    // Клавиши 1, 2, 3, ... загружают файлы из списка moleculeFiles.
    int index = key - GLFW_KEY_1;
    if (index >= 0 && index < MOLECULE_COUNT)
        loadXYZ(moleculeFiles[index], window);
}

// Перетаскивание файла в окно: загружаем его.
void dropCallback(GLFWwindow* window, int count, const char** paths) {
    if (count > 0) loadXYZ(paths[0], window);
}

// Изменение размера окна: обновляем область рисования и перспективу.
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    if (height < 1) height = 1;    // защита от деления на ноль
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float top = 0.1f * tan(45.0f * PI / 360.0f);   // угол обзора 45 градусов
    float right = top * width / height;            // учитываем пропорции окна
    glFrustum(-right, right, -top, top, 0.1, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    // 1. Инициализация GLFW
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // 2. Создание окна
    GLFWwindow* window = glfwCreateWindow(850, 650, "Molecules from XYZ", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 3. Инициализация GLAD (загрузка функций OpenGL)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        glfwTerminate();
        return -1;
    }

    // 4. Регистрация обработчиков событий
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetDropCallback(window, dropCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Dear ImGui: подключаем ПОСЛЕ наших обработчиков (true = ImGui встанет в цепочку
    // и будет вызывать наши обработчики после своих).
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::GetStyle().FontScaleMain = 1.7f;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    // Один раз настраиваем перспективу вручную (колбэк срабатывает только при изменении размера).
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    framebufferSizeCallback(window, width, height);

    // 5. Настройки OpenGL: тест глубины и простое освещение
    glEnable(GL_DEPTH_TEST);        // ближние объекты закрывают дальние
    glEnable(GL_LIGHTING);          // включаем освещение
    glEnable(GL_LIGHT0);            // включаем источник света 0
    glEnable(GL_COLOR_MATERIAL);    // glColor задаёт цвет материала
    glEnable(GL_NORMALIZE);         // нормали приводятся к единичной длине
    glClearColor(0.10f, 0.13f, 0.18f, 1.0f);

    // 6. Загрузка молекулы (имя файла из командной строки или water.xyz по умолчанию)
    loadXYZ(argc > 1 ? argv[1] : "water.xyz", window);

    // 7. Цикл отрисовки
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Очищаем экран и буфер глубины
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Свет задаём до поворотов, поэтому он неподвижен относительно камеры
        GLfloat light[] = { 5.0f, 8.0f, 10.0f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, light);

        // Отодвигаем камеру (масштаб) и применяем повороты мышью
        glTranslatef(0.0f, 0.0f, -cameraDistance / zoomLevel);
        glRotatef(rotX, 1.0f, 0.0f, 0.0f);
        glRotatef(rotY, 0.0f, 1.0f, 0.0f);

        // Сначала рисуем связи, потом атомы. Скрытые элементы пропускаем,
        // связи со скрытыми атомами тоже не рисуем.
        for (const auto& bond : bonds) {
            const Atom& a = atoms[bond.first];
            const Atom& b = atoms[bond.second];
            if (isHidden(a.element) || isHidden(b.element)) continue;
            drawBond(a, b);
        }

        for (const Atom& a : atoms) {
            if (isHidden(a.element)) continue;
            const ElementInfo& element = getElement(a.element);
            glPushMatrix();
            glTranslatef(a.x, a.y, a.z);                  // переносим в позицию атома
            glColor3f(element.r, element.g, element.b);   // цвет по таблице
            drawSphere(element.sphereRadius);             // размер по таблице
            glPopMatrix();
        }

        // Интерфейс поверх молекулы
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        drawLegend();
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        // Показываем нарисованный кадр
        glfwSwapBuffers(window);
    }

    // 8. Завершение работы
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}