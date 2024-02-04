#include <windows.h>
#include <cmath>
#include "Resource.h"
#include <algorithm>
#include <iostream>

#include <vector>
#include <string>
#include <sstream>
#include <numeric>
#include <fstream>

#include <random>
#include <ctime>

#include <codecvt>
#include <locale>

LRESULT CALLBACK KeyboardProc(int, WPARAM, LPARAM);
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

char szClassName[] = "KeyboardStatsWindow";
HHOOK hKeyboardHook;

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow) {
    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hPrevInstance, 0);
    HWND hwnd;
    MSG messages;
    WNDCLASSEX wincl;

    // Опис класу вікна
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style = CS_DBLCLKS;
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hIcon = (HICON)LoadImage(hThisInstance, "keyboard.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
    wincl.hIconSm = (HICON)LoadImage(hThisInstance, "keyboard.ico", IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    wincl.hbrBackground = CreatePatternBrush((HBITMAP)LoadImage(NULL, "background.bmp", IMAGE_BITMAP, 650, 480, LR_LOADFROMFILE | LR_DEFAULTSIZE));

    if (!RegisterClassEx(&wincl))
        return 0;

    hwnd = CreateWindowEx(
        0, szClassName, "Клавіатурний Почерк",
        WS_OVERLAPPEDWINDOW, 
        (GetSystemMetrics(SM_CXSCREEN) - 650) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - 480) / 2,
        650, 480, HWND_DESKTOP, NULL, hThisInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);

    while (GetMessage(&messages, NULL, 0, 0)) {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }

    UnhookWindowsHookEx(hKeyboardHook);
    return messages.wParam;
}


// Глобальні змінні


const char* staticText = "Робота повинна продемонструвати вміння "
"\n використовувати теоретичні знання.";
const char* hidePassword = "admin";



// Глобальні змінні
const double HIT_PERCENTAGE = 90.0;


struct KeyboardStats {
    bool isCapturing = false;
    ULONGLONG lastKeyDownTime = 0;
    ULONGLONG lastKeyUpTime = 0;
    std::vector<DWORD> keyDurations;
    std::vector<DWORD> intervalDurations;
    std::stringstream stringWithData;

    int deleteCount = 0;
};

KeyboardStats g_keyboardStats;



void OnKeyDown(KBDLLHOOKSTRUCT* pKeyInfo) {
    if (pKeyInfo->vkCode == VK_BACK || pKeyInfo->vkCode == VK_DELETE) {
        g_keyboardStats.deleteCount++;
    }
    if (g_keyboardStats.isCapturing && g_keyboardStats.lastKeyDownTime == 0) {


        ULONGLONG currentTime = GetTickCount64();
        if (g_keyboardStats.lastKeyUpTime != 0) {
            ULONGLONG interval = currentTime - g_keyboardStats.lastKeyUpTime; // Інтервал між натисканнями
            g_keyboardStats.intervalDurations.push_back(interval);
            // Тут можна зберегти interval для подальшого аналізу
        }
        g_keyboardStats.lastKeyDownTime = currentTime;
    }
}

void OnKeyUp() {
    if (g_keyboardStats.isCapturing) {
                ULONGLONG keyUpTime = GetTickCount64();
                ULONGLONG duration = keyUpTime - g_keyboardStats.lastKeyDownTime;
                g_keyboardStats.lastKeyDownTime = 0;
                g_keyboardStats.lastKeyUpTime = keyUpTime; // Оновлюємо час останнього відпускання клавіші
                g_keyboardStats.keyDurations.push_back(duration);
            }
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        if (wParam == WM_KEYDOWN) {   
            KBDLLHOOKSTRUCT* pKeyInfo = (KBDLLHOOKSTRUCT*)lParam;   
            OnKeyDown(pKeyInfo);
        }
        else if (wParam == WM_KEYUP) {
            OnKeyUp();
           
        }
    }
    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}



double globalRatio;
bool check(const std::vector<double>& numbersFromFile, std::vector<double> numbersFromStringStream) {
  
    if (numbersFromStringStream.size() != numbersFromFile.size()) {
        return false;  // Різний розмір векторів, неможливо порівнювати
    }

    double sum = 0.0;
    for (size_t i = 0; i < numbersFromStringStream.size() ; ++i) {
        double num1 = numbersFromStringStream[i];
        double num2 = numbersFromFile[i];
        double maxNum = max(num1, num2);
        double minNum = min(num1, num2);

        double ratio = (minNum / maxNum) * 100;
        sum += ratio;    
    }
    double meanRatio = sum / numbersFromFile.size();
    globalRatio = std::round(meanRatio * 10) / 10;
    return (meanRatio > HIT_PERCENTAGE);
}

void SaveDataToFile(const std::stringstream& ss) {
    std::ofstream outFile("User_Data.txt", std::ios::app); // Відкриття файлу для дописування
    if (outFile.is_open()) {
        outFile << ss.str() << "\n"; // Запис даних з нової строки
        outFile.close();
        MessageBox(NULL, "The data is saved.", "Information", MB_OK);
    }
    else {
        MessageBox(NULL, "Error while opening the file.", "Error", MB_OK);
    }
}


INT_PTR CALLBACK MyDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK: {
            // Отримання числа з елемента введення
            char buffer[256];
            GetDlgItemText(hwndDlg, IDC_EDIT1, buffer, 256);
            g_keyboardStats.stringWithData << " " << buffer;

            // Зберігання числа в глобальну змінну або передавання далі
            // ...
          
            EndDialog(hwndDlg, 0);
            return TRUE;
        }
        case IDCANCEL:
            EndDialog(hwndDlg, 0);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

INT_PTR CALLBACK MyDialogProc1(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK: {
            std::stringstream password;
            char buffer[256];
            GetDlgItemText(hwndDlg, IDC_EDIT1, buffer, 256);
            password << buffer;

            std::string ssString = password.str();
            if (ssString == hidePassword) {
                EndDialog(hwndDlg, TRUE); // Завершить диалог с TRUE, если пароль верный
            }
            else {
                MessageBox(NULL, "Wrong password", "Error", MB_OK);
            }
            return TRUE;
        }
        case IDCANCEL:
            EndDialog(hwndDlg, FALSE); // Завершить диалог с FALSE, если нажата кнопка отмены
            return TRUE;
        }
        break;
    }
    return FALSE;
}


void processFile(const std::string& filename, std::vector<double> numbersFromStringStream) {
std::stringstream matchingNames;
std::ifstream inFile(filename);
if (inFile.is_open()) {
    std::string line;
    while (std::getline(inFile, line)) {
        std::istringstream iss(line);
        std::vector<double> numbers;
        std::string name;

        double number;
        while (iss >> number) {
            numbers.push_back(number);
        }
        iss.clear();
        std::getline(iss, name);

        if (check(numbers, numbersFromStringStream)) {
            matchingNames << name << " - " << globalRatio << "%\n";
        }
    }

    if (!matchingNames.str().empty()) {
        MessageBox(NULL, matchingNames.str().c_str(), "Appropriate Names", MB_OK);
    }
    else {
        MessageBox(NULL, "No user found", "Result", MB_OK);
    }

    inFile.close();
}
else {
    MessageBox(NULL, "Failed to open file", "Error", MB_OK);
}

}

double calculateDeviation(double mean, std::vector<DWORD> data) {
    double relativeDeviationSum = 0.0;
    for (double val : data) {
        double deviation = (std::abs(val - mean) / mean) * 100.0;
        relativeDeviationSum += deviation;
    }

    double averageRelativeDeviation = relativeDeviationSum / data.size();

    return averageRelativeDeviation;
}

double calculateTotalTime() {
    double sum1 = std::accumulate(g_keyboardStats.keyDurations.begin(), g_keyboardStats.keyDurations.end(), 0.0);
    double sum2 = std::accumulate(g_keyboardStats.intervalDurations.begin(), g_keyboardStats.intervalDurations.end(), 0.0);
    size_t textLength = strlen(staticText);
    double avarageTime = (sum1 + sum2) / textLength;
    return avarageTime;
}


double calculateClearInput() {
    double procentClearInput;
    if (g_keyboardStats.deleteCount == 0) {
        procentClearInput = 100;
    }
    else {
        size_t textLength = strlen(staticText);
        if (textLength > 0) {
            procentClearInput = 100.0 - (static_cast<double>(g_keyboardStats.deleteCount) / textLength) * 100.0;
        }
        else {
            procentClearInput = 0; // или обработка ошибки, так как текст пустой
        }
    }
    return procentClearInput;
}

void calculateData(double totalTime) {
    double sum = std::accumulate(g_keyboardStats.keyDurations.begin(), g_keyboardStats.keyDurations.end(), 0.0);
    double mean = sum / g_keyboardStats.keyDurations.size();

    double sum1 = std::accumulate(g_keyboardStats.intervalDurations.begin(), g_keyboardStats.intervalDurations.end(), 0.0);
    double mean1 = sum1 / g_keyboardStats.intervalDurations.size();

    double deviation = calculateDeviation(mean, g_keyboardStats.keyDurations);
    double deviation1 = calculateDeviation(mean1, g_keyboardStats.intervalDurations);
    double allTime = calculateTotalTime();


    double procentClearInput = calculateClearInput();

    g_keyboardStats.stringWithData << " " << mean << " " << mean1 << " " << deviation << " " << deviation1 << " " << allTime << " " << totalTime << " " << procentClearInput;

}



std::vector<std::wstring> readLinesFromFile(const std::string& filename) {
    std::vector<std::wstring> lines;
    std::wstring line;
    std::wifstream file(filename);

    // Встановлення кодування UTF-8
    file.imbue(std::locale(file.getloc(), new std::codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>()));

    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    return lines;
}


// Функція для випадкового вибору рядка
std::wstring getRandomLine(const std::vector<std::wstring>& lines) {
    std::srand(std::time(nullptr)); // Ініціалізація генератора випадкових чисел
    int randomIndex = std::rand() % lines.size();
    return lines[randomIndex];
}


void clearData() {
    g_keyboardStats.stringWithData.str(""); // Очищаємо вміст
    g_keyboardStats.stringWithData.clear(); // Скидаємо стан потоку

    g_keyboardStats.deleteCount = 0;
    g_keyboardStats.keyDurations.clear();
    g_keyboardStats.intervalDurations.clear();
}


std::vector<double> ConvertStringStreamToVector(std::stringstream& ss) {
    std::vector<double> vec;
    double number;

    while (ss >> number) {
        vec.push_back(number);
    }

    return vec;
}
void HideAllChildWindows(HWND hwndParent)
{
    HWND hwndChild = GetWindow(hwndParent, GW_CHILD);
    while (hwndChild != NULL)
    {
        ShowWindow(hwndChild, SW_HIDE);
        hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
    }
}

HWND hEdit, hButton, hButtonSave, hButtonFind, hButtonIdentify, hButtonBack, hButtonAgain, hwndStatic, hwndStatic1, hwndStatic2, hButtons;
std::vector<std::wstring> lines;
 double totalTime;
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    lines = readLinesFromFile("textData.txt");
    std::wstring randomLine = getRandomLine(lines);
//    MessageBox(NULL, randomLine.c_str(), "Повідомлення", MB_OK);

    static LARGE_INTEGER frequency, startTime, endTime, accumulatedTime;
    static bool isTimerRunning = false;
    switch (message) {
    case WM_CREATE: {
        QueryPerformanceFrequency(&frequency);
        accumulatedTime.QuadPart = 0;
     
        hEdit = CreateWindow("EDIT", "",
            WS_CHILD |  WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL,
            195, 145, 260, 100, hwnd, (HMENU)1, NULL, NULL);
      

        hButton = CreateWindow("BUTTON", "Add User",
            WS_VISIBLE | WS_CHILD,
            80, 180, 180, 50, hwnd, (HMENU)2, NULL, NULL);

        hButtonIdentify = CreateWindow("BUTTON", "Identify User",
            WS_VISIBLE | WS_CHILD,
            355, 180, 200, 50, hwnd, (HMENU)5, NULL, NULL);

        hButtonSave = CreateWindow("BUTTON", "Save",
            WS_CHILD,
            350, 265, 120, 60, hwnd, (HMENU)3, NULL, NULL);

        hButtonBack = CreateWindow("BUTTON", "Go Back",
            WS_CHILD,
            350, 350, 120, 60, hwnd, (HMENU)6, NULL, NULL);

        hButtonAgain = CreateWindow("BUTTON", "Retry",
            WS_CHILD,
            175, 350, 120, 60, hwnd, (HMENU)7, NULL, NULL);

        hButtonFind = CreateWindow("BUTTON", "Find",
            WS_CHILD,
            350, 265, 120, 60, hwnd, (HMENU)4, NULL, NULL);

        hwndStatic = CreateWindowExW(0, L"STATIC", randomLine.c_str(),
            WS_CHILD | SS_CENTER, 20, 60, 650, 70,
            hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        hwndStatic1 = CreateWindow("STATIC", "Enter the next text",
            WS_CHILD | SS_CENTER, 210, 15, 250, 40,
            hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        hwndStatic2 = CreateWindow("STATIC", "Time",
            WS_CHILD | SS_CENTER, 200, 100, 250, 40,
            hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        hButtons = CreateWindow("BUTTON", "Start",
            WS_CHILD,
            175, 265, 120, 60, hwnd, (HMENU)10, NULL, NULL);

    
        break;
    }
 
    case WM_CTLCOLORSTATIC:
    {
        // установка белого цвета фона
        SetBkColor((HDC)wParam, RGB(255, 255, 255));
        return (LRESULT)GetStockObject(WHITE_BRUSH);
    }

    
    case WM_TIMER: {
        if (wParam == 1) {
            LARGE_INTEGER currentTime;
            QueryPerformanceCounter(&currentTime);
            accumulatedTime.QuadPart += currentTime.QuadPart - startTime.QuadPart;
            startTime = currentTime;  // Обновляем startTime для следующего тика

            double timeElapsed = static_cast<double>(accumulatedTime.QuadPart) / frequency.QuadPart;
            timeElapsed = std::round(timeElapsed * 10) / 10;
            totalTime = timeElapsed;
            std::stringstream stime;
            stime << "Час: " << timeElapsed << " секунд";
            SetWindowText(hwndStatic2, stime.str().c_str());
        }
        break;
    }



    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);

        // Проверяем изменение текста в многострочном поле ввода
        if (wmId == 1 && wmEvent == EN_CHANGE) {
            if (!g_keyboardStats.isCapturing) {
                MessageBox(hwnd, "First press the button 'Start'", "Warning", MB_OK);
                // Очищаем текст в поле ввода
                SetWindowText(hEdit, "");
            }
        }

        if (LOWORD(wParam) == 10) {
             g_keyboardStats.isCapturing = !g_keyboardStats.isCapturing;
            SetWindowText(hButtons, g_keyboardStats.isCapturing ? "Stop" : "Start");

            if (g_keyboardStats.isCapturing) {
                SetFocus(hEdit);
                QueryPerformanceCounter(&startTime);
                SetTimer(hwnd, 1, 100, NULL); // Запускаем таймер с интервалом 1 секунда

                EnableWindow(hButtonSave, FALSE);
                EnableWindow(hButtonFind, FALSE);
                EnableWindow(hButtonBack, FALSE);
                EnableWindow(hButtonAgain, FALSE);
            } else {
                QueryPerformanceCounter(&endTime);
                accumulatedTime.QuadPart += endTime.QuadPart - startTime.QuadPart; // Сохраняем прошедшее время
                KillTimer(hwnd, 1);
              
                g_keyboardStats.lastKeyDownTime = 0;
                g_keyboardStats.lastKeyUpTime = 0;
                EnableWindow(hButtonSave, TRUE);
                EnableWindow(hButtonFind, TRUE);
                EnableWindow(hButtonBack, TRUE);
                EnableWindow(hButtonAgain, TRUE);
            }

          
        }


        if (LOWORD(wParam) == 4) {

            if (!g_keyboardStats.keyDurations.empty() && !g_keyboardStats.intervalDurations.empty()) {
                calculateData(totalTime);
                std::vector<double> numbers = ConvertStringStreamToVector(g_keyboardStats.stringWithData);

                processFile("User_Data.txt", numbers);

                SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)L"");

                clearData();
                SetWindowTextW(hwndStatic, getRandomLine(lines).c_str());
                accumulatedTime.QuadPart = 0;
                if (g_keyboardStats.isCapturing) {
                    QueryPerformanceCounter(&startTime); // Сбросить startTime, если таймер активен
                }
                else {
                    SetWindowText(hwndStatic2, "Time: 0 s");
                }
            } else
            MessageBox(NULL, "Enter text", "Error", MB_OK);
        }
   
        if (LOWORD(wParam) == 3) {
            if (!g_keyboardStats.keyDurations.empty() && !g_keyboardStats.intervalDurations.empty()) {

                KillTimer(hwnd, 1);
                calculateData(totalTime);

                DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG1), hwnd, MyDialogProc);

                SaveDataToFile(g_keyboardStats.stringWithData);

                clearData();

                SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)L"");
                SetWindowTextW(hwndStatic, getRandomLine(lines).c_str());
                accumulatedTime.QuadPart = 0;
                if (g_keyboardStats.isCapturing) {
                    QueryPerformanceCounter(&startTime); // Сбросить startTime, если таймер активен
                }
                else {
                    SetWindowText(hwndStatic2, "Time: 0 s");
                }

            } else 
                MessageBox(NULL, "Enter text", "Error", MB_OK);
        }
 

        if (LOWORD(wParam) == 2) {
            // Вызов DialogBox
            INT_PTR dialogResult = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG2), hwnd, MyDialogProc1);

            // Проверка результатов DialogBox
            if (dialogResult == TRUE) {
                // Эти действия выполняются, если DialogBox завершился с TRUE
                HideAllChildWindows(hwnd);
                ShowWindow(hEdit, SW_SHOW);
                ShowWindow(hButtonSave, SW_SHOW);
                ShowWindow(hButtonBack, SW_SHOW);
                ShowWindow(hButtonAgain, SW_SHOW);
                ShowWindow(hwndStatic, SW_SHOW);
                ShowWindow(hwndStatic1, SW_SHOW);
                ShowWindow(hwndStatic2, SW_SHOW);
                ShowWindow(hButtons, SW_SHOW);

                SetWindowTextW(hwndStatic, getRandomLine(lines).c_str());
                accumulatedTime.QuadPart = 0;
                if (g_keyboardStats.isCapturing) {
                    QueryPerformanceCounter(&startTime); // Сбросить startTime, если таймер активен
                }
                else {
                    SetWindowText(hwndStatic2, "Time: 0 s");
                }
            }


        }


        if (LOWORD(wParam) == 5) {
            HideAllChildWindows(hwnd);
            ShowWindow(hEdit, SW_SHOW);
            ShowWindow(hButtonFind, SW_SHOW);
            ShowWindow(hButtonBack, SW_SHOW);
            ShowWindow(hwndStatic, SW_SHOW);
            SetWindowTextW(hwndStatic, getRandomLine(lines).c_str());
            ShowWindow(hwndStatic1, SW_SHOW);
            ShowWindow(hwndStatic2, SW_SHOW);
            ShowWindow(hButtons, SW_SHOW);
            ShowWindow(hButtonAgain, SW_SHOW);

        }


        if (LOWORD(wParam) == 6) {
            clearData();
            SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)L"");
            HideAllChildWindows(hwnd);
            ShowWindow(hButton, SW_SHOW);
            ShowWindow(hButtonIdentify, SW_SHOW);
            accumulatedTime.QuadPart = 0;
            if (g_keyboardStats.isCapturing) {
                QueryPerformanceCounter(&startTime); // Сбросить startTime, если таймер активен
            }
            else {
                SetWindowText(hwndStatic2, "Time: 0 s");
            }

        }


        if (LOWORD(wParam) == 7) {
            clearData();

            SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)L"");
           // std::wstring randomLine = getRandomLine(lines);
            SetWindowTextW(hwndStatic, getRandomLine(lines).c_str());

            accumulatedTime.QuadPart = 0;
            if (g_keyboardStats.isCapturing) {
                QueryPerformanceCounter(&startTime); // Сбросить startTime, если таймер активен
            }
            else {
                SetWindowText(hwndStatic2, "Time: 0 s");
            }

        }

        break;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }
    default: return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}
