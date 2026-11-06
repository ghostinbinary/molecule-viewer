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

// Настройки отображения и поиска связей
const float BOND_THICKNESS = 0.09f;
const float BOND_TOLERANCE = 1.25f;  // Допустимое расстояние с учётом радиусов атомов
const float BOND_RELATIVE = 1.30f;  // Отсекаем связи, которые слишком длинные относительно ближайшей
const float MOUSE_SENSITIVITY = 0.5f;
const float MIN_ZOOM = 0.3f;
const float MAX_ZOOM = 4.0f;
const int SPHERE_STEPS = 20;  // Детализация сфер
const int BOND_STEPS = 12;  // Детализация цилиндров

// Радиусы и цвета элементов
struct ElementInfo {
    const char* symbol;
    float bondRadius;  // Ковалентный радиус для поиска связей
    float sphereRadius;  // Радиус сферы при отрисовке
    float r, g, b;  // Цвет RGB
};

const ElementInfo elements[] = {
    // Символ  радиус связи  размер шара  цвет RGB
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
    { "Na",    1.66f, 0.55f,  0.67f, 0.36f, 0.95f },
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

// Для элементов, которых нет в таблице
const ElementInfo defaultElement = { "?", 0.75f, 0.42f, 0.65f, 0.55f, 0.75f };

const ElementInfo& getElement(const string& symbol) {
    for (const ElementInfo& element : elements)
        if (symbol == element.symbol) return element;
    return defaultElement;
}

// Молекулы, доступные по клавишам 1–6
const char* moleculeFiles[] = {
    "water.xyz", "methane.xyz", "benzene.xyz",
    "caffeine.xyz", "ammonia.xyz", "bromomethane.xyz"
};
const int MOLECULE_COUNT = sizeof(moleculeFiles) / sizeof(moleculeFiles[0]);

// Атомы и связи текущей молекулы
struct Atom {
    string element;
    float x, y, z;
};

vector<Atom> atoms;
vector<pair<int, int>> bonds;  // Индексы двух соединённых атомов

vector<string> usedElements;  // Элементы для легенды
set<string> hiddenElements;  // Скрытые через легенду элементы

bool isHidden(const string& symbol) {
    return hiddenElements.count(symbol) > 0;
}

// Поворот и масштабирование мышью
bool isDragging = false;
double lastMouseX = 0.0;
double lastMouseY = 0.0;
float rotX = 20.0f;
float rotY = 25.0f;
float zoomLevel = 1.0f;
float cameraDistance = 8.0f;

// Металлы, для которых исключаем связи между одинаковыми атомами
bool isMetal(const string& s) {
    static const set<string> metals = {
        // Щелочные и щелочноземельные металлы
        "Li", "Na", "K", "Rb", "Cs", "Fr",
        "Be", "Mg", "Ca", "Sr", "Ba", "Ra",

        // Переходные металлы 
        "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn",
        "Y", "Zr", "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd",
        "Hf", "Ta", "W", "Re", "Os", "Ir", "Pt", "Au", "Hg",
        "Rf", "Db", "Sg", "Bh", "Hs", "Mt", "Ds", "Rg", "Cn",

    };

    return metals.count(s) > 0;
}

// XYZ: число атомов, строка описания, затем символ элемента и координаты
bool loadXYZ(const string& filename, GLFWwindow* window) {
    ifstream file(filename);

    // Если файл не найден по указанному пути, ищем его в папке molecules рядом с программой
    if (!file.is_open()) {
        file.clear();
        file.open("molecules/" + filename);
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
    getline(file, comment);  // Переходим на следующую строку
    getline(file, comment);  // Пропускаем описание молекулы

    // Читаем во временный массив, чтобы при ошибке сохранить прежнюю молекулу
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

    // Собираем уникальные элементы для легенды в порядке появления
    // При загрузке новой молекулы снова показываем все элементы
    usedElements.clear();
    hiddenElements.clear();
    for (const Atom& a : atoms) {
        bool found = false;
        for (const string& s : usedElements)
            if (s == a.element) { found = true; break; }
        if (!found) usedElements.push_back(a.element);
    }

    // Смещаем молекулу так, чтобы её центр был в начале координат
    float cx = 0, cy = 0, cz = 0;
    for (const Atom& a : atoms) { cx += a.x; cy += a.y; cz += a.z; }
    cx /= count; cy /= count; cz /= count;

    // Находим расстояние до самого дальнего атома
    float size = 0;
    for (Atom& a : atoms) {
        a.x -= cx; a.y -= cy; a.z -= cz;
        float d = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
        if (d > size) size = d;
    }
    // Подбираем расстояние до камеры под размер молекулы
    cameraDistance = (size + 1.0f) * 3.0f;
    zoomLevel = 1.0f;

    // Ищем связи по расстояниям и ковалентным радиусам
    // Для каждого атома запоминаем самую короткую связь относительно суммы радиусов
    // ratio — расстояние между атомами, делённое на сумму их радиусов
    struct Candidate { int i, j; float ratio; };
    vector<Candidate> candidates;
    vector<float> bestRatio(count, 1000.0f);

    for (int i = 0; i < count; ++i) {
        for (int j = i + 1; j < count; ++j) {
            if (atoms[i].element == atoms[j].element && isMetal(atoms[i].element)) continue;

            // Запрещаем связи между атомами Mo
            if (atoms[i].element == "Mo" && atoms[j].element == "Mo")
                continue; 

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

    // Убираем кандидатов, которые заметно длиннее ближайших связей обоих атомов
    bonds.clear();
    for (const Candidate& c : candidates) {
        if (c.ratio <= bestRatio[c.i] * BOND_RELATIVE &&
            c.ratio <= bestRatio[c.j] * BOND_RELATIVE)
            bonds.push_back({ c.i, c.j });
    }

    // Показываем имя файла и количество атомов и связей в заголовке
    string title = filename + " - " + to_string(count) +
        " atoms, " + to_string(bonds.size()) + " bonds";
    glfwSetWindowTitle(window, title.c_str());
    return true;
}

// Рисуем сферу полосами между соседними параллелями
void drawSphere(float radius) {
    for (int i = 0; i < SPHERE_STEPS; ++i) {
        float lat1 = -PI / 2 + PI * i / SPHERE_STEPS;
        float lat2 = -PI / 2 + PI * (i + 1) / SPHERE_STEPS;
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= SPHERE_STEPS; ++j) {
            float lon = 2 * PI * j / SPHERE_STEPS;
            float x = cos(lon), z = sin(lon);
            // Нормали нужны для правильного освещения
            glNormal3f(cos(lat1) * x, sin(lat1), cos(lat1) * z);
            glVertex3f(radius * cos(lat1) * x, radius * sin(lat1), radius * cos(lat1) * z);
            glNormal3f(cos(lat2) * x, sin(lat2), cos(lat2) * z);
            glVertex3f(radius * cos(lat2) * x, radius * sin(lat2), radius * cos(lat2) * z);
        }
        glEnd();
    }
}

// Связь между атомами рисуем цилиндром
void drawBond(const Atom& a, const Atom& b) {
    float dx = b.x - a.x, dy = b.y - a.y, dz = b.z - a.z;
    float length = sqrt(dx * dx + dy * dy + dz * dz);
    if (length < 0.001f) return;

    glPushMatrix();
    glTranslatef(a.x, a.y, a.z);
    // Поворачиваем цилиндр, построенный вдоль Z, к следующему атому
    float angle = acos(dz / length) * 180.0f / PI;
    if (fabs(dx) + fabs(dy) > 0.0001f)
        glRotatef(angle, -dy, dx, 0);
    else if (dz < 0) glRotatef(180, 1, 0, 0);  // Отдельный случай: направление строго по отрицательной оси Z

    glColor3f(0.70f, 0.70f, 0.73f);
    glBegin(GL_QUAD_STRIP);
    for (int k = 0; k <= BOND_STEPS; ++k) {
        float t = 2 * PI * k / BOND_STEPS, x = cos(t), y = sin(t);
        glNormal3f(x, y, 0);
        glVertex3f(BOND_THICKNESS * x, BOND_THICKNESS * y, 0);
        glVertex3f(BOND_THICKNESS * x, BOND_THICKNESS * y, length);
    }
    glEnd();
    glPopMatrix();
}

// Кнопка видимости элемента в легенде
bool eyeButton(const char* id, bool visible) {
    ImVec2 size(28.0f, ImGui::GetTextLineHeight());
    ImVec2 p = ImGui::GetCursorScreenPos();
    bool clicked = ImGui::InvisibleButton(id, size);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImU32 col = visible ? IM_COL32(235, 235, 235, 255) : IM_COL32(120, 120, 120, 255);
    float cx = p.x + size.x * 0.5f, cy = p.y + size.y * 0.5f;
    float w = size.x * 0.5f, h = size.y * 0.5f;
    ImVec2 left(cx - w + 2, cy), right(cx + w - 2, cy);

    // Контур глаза и зрачок
    dl->AddBezierQuadratic(left, ImVec2(cx, cy - h * 1.6f), right, col, 1.5f);
    dl->AddBezierQuadratic(left, ImVec2(cx, cy + h * 1.6f), right, col, 1.5f);
    dl->AddCircleFilled(ImVec2(cx, cy), h * 0.45f, col);
    if (!visible)
        dl->AddLine(ImVec2(cx - w + 4, cy + h), ImVec2(cx + w - 4, cy - h), col, 2.0f);
    return clicked;
}

// Легенда с цветами и переключателями видимости
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

        // Скрытые элементы рисуем полупрозрачными
        float h = ImGui::GetTextLineHeight();
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImU32 fill = IM_COL32((int)(info.r * 255), (int)(info.g * 255), (int)(info.b * 255), visible ? 255 : 70);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddCircleFilled(ImVec2(p.x + h * 0.5f, p.y + h * 0.5f), h * 0.5f, fill);
        dl->AddCircle(ImVec2(p.x + h * 0.5f, p.y + h * 0.5f), h * 0.5f, IM_COL32(200, 200, 200, visible ? 200 : 70));
        ImGui::Dummy(ImVec2(h, h));

        ImGui::SameLine();
        if (visible) ImGui::Text("%s", symbol.c_str());
        else         ImGui::TextDisabled("%s", symbol.c_str());

        ImGui::SameLine(75.0f);
        if (eyeButton("##eye", visible)) {
            if (visible) hiddenElements.insert(symbol);
            else         hiddenElements.erase(symbol);
        }
        ImGui::PopID();
    }
    ImGui::End();
}

// Обработка мыши и клавиатуры

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            // Клики по интерфейсу не должны вращать молекулу
            if (ImGui::GetIO().WantCaptureMouse) return;
            isDragging = true;
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
        }
        else if (action == GLFW_RELEASE) {
            isDragging = false;
        }
    }
}

// Вращаем молекулу при перетаскивании мышью
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (isDragging) {
        double deltaX = xpos - lastMouseX;
        double deltaY = ypos - lastMouseY;

        rotY += (float)deltaX * MOUSE_SENSITIVITY;
        rotX += (float)deltaY * MOUSE_SENSITIVITY;

        lastMouseX = xpos;
        lastMouseY = ypos;
    }
}

// Масштабирование колёсиком мыши
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (ImGui::GetIO().WantCaptureMouse) return;
    zoomLevel *= yoffset > 0 ? 1.1f : 0.9f;
    if (zoomLevel < MIN_ZOOM) zoomLevel = MIN_ZOOM;
    if (zoomLevel > MAX_ZOOM) zoomLevel = MAX_ZOOM;
}

// ESC закрывает окно, клавиши 1–6 переключают молекулы
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(window, true);

    int index = key - GLFW_KEY_1;
    if (index >= 0 && index < MOLECULE_COUNT)
        loadXYZ(moleculeFiles[index], window);
}

// Загрузка XYZ перетаскиванием в окно
void dropCallback(GLFWwindow* window, int count, const char** paths) {
    if (count > 0) loadXYZ(paths[0], window);
}

// Обновляем перспективу при изменении размера окна
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    if (height < 1) height = 1;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float top = 0.1f * tan(45.0f * PI / 360.0f);  // Угол обзора 45 градусов
    float right = top * width / height;
    glFrustum(-right, right, -top, top, 0.1, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    // Запуск GLFW и создание окна OpenGL 2.1
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = glfwCreateWindow(850, 650, "Molecules from XYZ", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        glfwTerminate();
        return -1;
    }

    // Подключаем обработчики ввода
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetDropCallback(window, dropCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // ImGui подключается с передачей событий нашим обработчикам
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui::GetStyle().FontScaleMain = 1.7f;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    // Задаём перспективу при запуске, до первого изменения размера окна
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    framebufferSizeCallback(window, width, height);

    // Глубина, освещение и цвета материалов
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glClearColor(0.10f, 0.13f, 0.18f, 1.0f);

    // По умолчанию открываем воду, либо файл из аргументов запуска
    loadXYZ(argc > 1 ? argv[1] : "water.xyz", window);

    // Основной цикл
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Свет остаётся на месте при вращении молекулы
        GLfloat light[] = { 5.0f, 8.0f, 10.0f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, light);

        glTranslatef(0.0f, 0.0f, -cameraDistance / zoomLevel);
        glRotatef(rotX, 1.0f, 0.0f, 0.0f);
        glRotatef(rotY, 0.0f, 1.0f, 0.0f);

        // Не рисуем связи, если хотя бы один из атомов скрыт
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
            glTranslatef(a.x, a.y, a.z);
            glColor3f(element.r, element.g, element.b);
            drawSphere(element.sphereRadius);
            glPopMatrix();
        }

        // Рисуем легенду поверх 3D-сцены
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        drawLegend();
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Освобождаем ресурсы ImGui и GLFW
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
