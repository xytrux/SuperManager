#include <windows.h>
#include <stdio.h>
#include <lmcons.h>
#include <psapi.h>
#include <tlhelp32.h>

#define USER_INFO u->pw_gecos
#define HOME_DIR u->pw_dir

HWND hOut;

static BOOL isSystemInfoDisplayed = FALSE;
static BOOL isProcessesDisplayed = FALSE;

void appendToEditControl(HWND hOut, const char *text)
{
    int TextLen = GetWindowTextLength(hOut);
    SendMessage(hOut, EM_SETSEL, (WPARAM)TextLen, (LPARAM)TextLen); // set the selection at the end of text
    SendMessage(hOut, EM_REPLACESEL, 0, (LPARAM)text);              // replace the selection
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    char buffer[128];
    if (IsWindowVisible(hwnd))
    {
        GetWindowText(hwnd, buffer, sizeof(buffer));
        if (strlen(buffer) != 0)
        {
            strcat((char *)lParam, "   - ");
            strcat((char *)lParam, buffer);
            strcat((char *)lParam, "\r\n");
        }
    }
    return TRUE;
}

void printUserInfo()
{
    SendMessage(hOut, WM_SETTEXT, 0, (LPARAM) "");
    TCHAR username[UNLEN + 1];
    DWORD size = UNLEN + 1;
    if (GetUserName((TCHAR *)username, &size))
    {
        char output[512];
        sprintf(output, "User Info:\r\n\r\nUsername: %s\r\n", username);
        appendToEditControl(hOut, output);
    }
}

void printSystemInfo(SYSTEM_INFO *s)
{
    SendMessage(hOut, WM_SETTEXT, 0, (LPARAM) "");
    char output[512];
    sprintf(output, "System Info:\r\n"
                    "Processor Architecture: %d\r\n"
                    "Page size: %d\r\n"
                    "Processor type: %d\r\n"
                    "Number of processors: %d\r\n"
                    "Minimum application address: %ld\r\n"
                    "Maximum application address: %ld\r\n"
                    "Active processor mask: %ld\r\n\r\n",
            s->wProcessorArchitecture,
            s->dwPageSize,
            s->dwProcessorType,
            s->dwNumberOfProcessors,
            s->lpMinimumApplicationAddress,
            s->lpMaximumApplicationAddress,
            s->dwActiveProcessorMask);
    appendToEditControl(hOut, output);
}

void listOpenWindows()
{
    SendMessage(hOut, WM_SETTEXT, 0, (LPARAM) "");
    char output[4096] = "Open Windows:\r\n\r\n";
    EnumWindows(EnumWindowsProc, (LPARAM)output);
    appendToEditControl(hOut, output);
}

void listFilesAndFolders()
{
    SendMessage(hOut, WM_SETTEXT, 0, (LPARAM) "");
    char output[512] = "File Manager:\r\n\r\n";

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(".\\*", &findFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            strcat(output, "   - ");
            strcat(output, findFileData.cFileName);
            strcat(output, "\r\n");
        } while (FindNextFile(hFind, &findFileData));
        FindClose(hFind);
    }

    appendToEditControl(hOut, output);
}

void printHelp()
{
    SendMessage(hOut, WM_SETTEXT, 0, (LPARAM) "");
    char output[512] = "Help:\r\n   - Instructions on how to use each feature.\r\n   - Detailed explanations for each tab.\r\n";
    appendToEditControl(hOut, output);
}

void listProcesses()
{
    SendMessage(hOut, WM_SETTEXT, 0, (LPARAM) "");
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    char output[4096] = "Running Processes:\r\n";

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        return;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32))
    {
        CloseHandle(hProcessSnap);
        return;
    }

    do
    {
        strcat(output, "   - ");
        strcat(output, pe32.szExeFile);
        strcat(output, "\r\n");
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);

    appendToEditControl(hOut, output);
}

void launchProgram(const char *program)
{
    ShellExecute(NULL, "open", program, NULL, NULL, SW_SHOWDEFAULT);
}

typedef struct {
    SYSTEM_INFO sysInfo;
    MEMORYSTATUSEX memInfo;
    DWORD uptime, processes;
} MY_SYSTEM_INFO;

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    MY_SYSTEM_INFO s;
    s.memInfo.dwLength = sizeof(s.memInfo);
    GlobalMemoryStatusEx(&(s.memInfo));
    s.uptime = GetTickCount() / 1000;
    s.processes = 0;

    switch (msg)
    {
    case WM_CREATE:
    {
        CreateWindow("BUTTON", "User Info", WS_VISIBLE | WS_CHILD, 10, 10, 100, 30, hwnd, (HMENU)1, NULL, NULL);
        CreateWindow("BUTTON", "System Info", WS_VISIBLE | WS_CHILD, 10, 50, 100, 30, hwnd, (HMENU)2, NULL, NULL);
        CreateWindow("BUTTON", "List Files", WS_VISIBLE | WS_CHILD, 10, 90, 100, 30, hwnd, (HMENU)3, NULL, NULL);
        CreateWindow("BUTTON", "Open Notepad", WS_VISIBLE | WS_CHILD, 10, 130, 100, 30, hwnd, (HMENU)4, NULL, NULL);
        CreateWindow("BUTTON", "Open Calc", WS_VISIBLE | WS_CHILD, 10, 170, 100, 30, hwnd, (HMENU)5, NULL, NULL);
        CreateWindow("BUTTON", "Open Cmd", WS_VISIBLE | WS_CHILD, 10, 210, 100, 30, hwnd, (HMENU)6, NULL, NULL);
        CreateWindow("BUTTON", "Help", WS_VISIBLE | WS_CHILD, 10, 250, 100, 30, hwnd, (HMENU)7, NULL, NULL);
        CreateWindow("BUTTON", "List Processes", WS_VISIBLE | WS_CHILD, 10, 290, 100, 30, hwnd, (HMENU)8, NULL, NULL);
        CreateWindow("BUTTON", "List Windows", WS_VISIBLE | WS_CHILD, 10, 330, 100, 30, hwnd, (HMENU)9, NULL, NULL);
        hOut = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY, 120, 10, 440, 340, hwnd, NULL, NULL, NULL);
        SetTimer(hwnd, 10, 1000, NULL);
        break;
    }
    case WM_COMMAND:
    {
        switch (wp)
        {
        case 1:
            printUserInfo();
            break;
        case 2:
            GetSystemInfo(&(s.sysInfo));
            printSystemInfo(&(s.sysInfo));
            break;
        case 3:
            listFilesAndFolders();
            break;
        case 4:
            launchProgram("notepad");
            break;
        case 5:
            launchProgram("calc");
            break;
        case 6:
            launchProgram("cmd");
            break;
        case 7:
            printHelp();
            break;
        case 8:
            listProcesses();
            break;
        case 9:
            listOpenWindows();
            break;
        }

        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_TIMER:
        if (isSystemInfoDisplayed) {
        GetSystemInfo(&(s.sysInfo));
        printSystemInfo(&(s.sysInfo));
    }
    if (isProcessesDisplayed) {
        listProcesses();
    }
        break;
    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const char CLASS_NAME[] = "SuperManagerWindowClass";

    WNDCLASS wc = {0};

    wc.lpfnWndProc = WindowProcedure;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, "Window registration failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    int width = 600;
    int height = 600;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int posX = (screenWidth - width) / 2;
    int posY = (screenHeight - height) / 2;

    HWND hwnd = CreateWindow(CLASS_NAME, "SuperManager", WS_OVERLAPPEDWINDOW, posX, posY, width, height, NULL, NULL, hInst, NULL);

    if (hwnd == NULL)
    {
        MessageBox(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}