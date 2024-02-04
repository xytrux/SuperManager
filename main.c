#include <windows.h>
#include <stdio.h>
#include <lmcons.h>
#include <psapi.h>
#include <tlhelp32.h>

#define USER_INFO u->pw_gecos
#define HOME_DIR u->pw_dir
#define SYSTEM_INFO struct SystemInfo

SYSTEM_INFO
{
    MEMORYSTATUSEX memInfo;
    DWORD uptime, processes;
};

HWND hOut;

void printUserInfo()
{
    TCHAR username[UNLEN + 1];
    DWORD size = UNLEN + 1;
    if (GetUserName((TCHAR *)username, &size))
    {
        char output[512];
        sprintf(output, "User Info:\n   - Username: %s\n\n", username);
        SendMessage(hOut, EM_SETSEL, (WPARAM)-1, (LPARAM)-1); // Set selection to end of text
        SendMessage(hOut, EM_REPLACESEL, 0, (LPARAM)output);  // Insert new text at selection
    }
}

void printSystemInfo(SYSTEM_INFO *s)
{
    char output[512];
    sprintf(output, "System Info:\n   - Uptime: %ld seconds\n   - Total RAM: %.2f MB (%.2f GB)\n   - Free RAM: %.2f MB (%.2f GB)\n   - Processes: %d\n\n",
            s->uptime,
            (float)s->memInfo.ullTotalPhys / (1024 * 1024), (float)s->memInfo.ullTotalPhys / (1024 * 1024 * 1024),
            (float)s->memInfo.ullAvailPhys / (1024 * 1024), (float)s->memInfo.ullAvailPhys / (1024 * 1024 * 1024),
            s->processes);
    SetWindowText(hOut, output);
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
            strcat((char *)lParam, "\n");
        }
    }
    return TRUE;
}

void listOpenWindows()
{
    char output[4096] = "Open Windows:\n";
    EnumWindows(EnumWindowsProc, (LPARAM)output);
    SetWindowText(hOut, output);
}

void listFilesAndFolders()
{
    char output[512] = "File Manager:\n";

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(".\\*", &findFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            strcat(output, "   - ");
            strcat(output, findFileData.cFileName);
            strcat(output, "\n");
        } while (FindNextFile(hFind, &findFileData));
        FindClose(hFind);
    }

    SetWindowText(hOut, output);
}

void launchProgram(const char *program)
{
    ShellExecute(NULL, "open", program, NULL, NULL, SW_SHOWDEFAULT);
}

void printHelp()
{
    char output[512] = "Help:\n   - Instructions on how to use each feature.\n   - Detailed explanations for each tab.\n";
    SetWindowText(hOut, output);
}

void listProcesses()
{
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    char output[4096] = "Running Processes:\n";

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        return;
    }

    // Set the size of the structure before using it.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process,
    // and exit if unsuccessful
    if (!Process32First(hProcessSnap, &pe32))
    {
        CloseHandle(hProcessSnap); // clean the snapshot object
        return;
    }

    // Now walk the snapshot of processes
    do
    {
        strcat(output, "   - ");
        strcat(output, pe32.szExeFile);
        strcat(output, "\n");
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);

    SetWindowText(hOut, output);
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    SYSTEM_INFO s;
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
            printSystemInfo(&s);
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
    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow)
{
    WNDCLASSW wc = {0};

    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hInst;
    wc.lpszClassName = L"myWindowClass";
    wc.lpfnWndProc = WindowProcedure;

    if (!RegisterClassW(&wc))
        return -1;

    CreateWindowW(L"myWindowClass", L"SuperManager", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, 600, 600, NULL, NULL, NULL, NULL);

    MSG msg = {0};

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}