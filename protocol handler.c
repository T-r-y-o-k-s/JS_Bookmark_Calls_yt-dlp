#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>>
#include <windows.h>

void runProcess(char *cmdline) {
    STARTUPINFO siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(siStartInfo));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    
    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(piProcInfo));
    
    
    CreateProcessA(NULL, cmdline, NULL, NULL, true, 0, NULL, NULL, &siStartInfo, &piProcInfo); 
    WaitForSingleObject(piProcInfo.hProcess, INFINITE);
    
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    
    return;
}

char *substr(char *instr, int pos, int len) {
    instr[pos + len] = '\0';
    return &instr[pos];
}

int main(int argc, char** argv) { // argv[1] = yt-dlp://R-DEp62qDeE/
    if (argc != 2) {
        printf("This program is intended to only be executed via the yt-dlp:// protocol.\n");
        printf("If this is actually the case, then there is a bug in here somewhere ...\n");
        return 1;
    }
    
    if (strlen(argv[1]) < 9) {
        printf("There probably is no video ID provided.\n");
        printf("Check the javascript code in the bookmark for errors.\n");
        return 2;
    }
    
    char *instr = argv[1];
    int pos = 9;
    int len = strlen(instr) - pos - 1;
    char *id = substr(instr, pos, len);
    
    char url[128] = "https://www.youtube.com/watch?v=";
    strcat(url, id);
    
    char *exec = "\"vlc --qt-start-minimized --start-paused --play-and-exit {} && del {}\"";
    
    char cmdline[1024] = "yt-dlp -f \"bv[ext=mp4][height<=1080]+ba/best\" -o %USERPROFILE%/Videos/yt-dlp_tmp/%(title)s.%(ext)s -R infinite --exec ";
    strcat(cmdline, exec);
    strcat(cmdline, " \""); strcat(cmdline, url); strcat(cmdline, "\"");
    
    printf("Running [%s]\n\n", cmdline);
    runProcess(cmdline);
    return 0;
}