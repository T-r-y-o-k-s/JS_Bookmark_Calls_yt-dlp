#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

#define STR_SZ 128

#define GOTOX "\x1b[" "%d" "G" // %d needs to be specified wherever this is used
#define GOUP "\033[1F"
#define GODOWN "\033[1E"
#define BOLD "\033[1m"
#define UNDERLINED "\033[4m"
#define RED "\033[31m"
#define RESET "\033[0m"

int runProcess(char *cmdline, bool wait) {
    STARTUPINFO siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(siStartInfo));
    siStartInfo.cb = sizeof(STARTUPINFO);
    // siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    // siStartInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    // siStartInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    // siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    
    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(piProcInfo));
        
    if (!CreateProcessA(NULL, cmdline, NULL, NULL, true, 0, NULL, NULL, &siStartInfo, &piProcInfo)) {
        printf("Couldn't create new process: %lu\n", GetLastError());
        return -1;
    }
    if (wait == false) return 0;
    WaitForSingleObject(piProcInfo.hProcess, INFINITE); // error handling
    
    int ret;
    GetExitCodeProcess(piProcInfo.hProcess, (LPDWORD)&ret);
    
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    
    return ret;
}

char *substr(char *instr, int pos, int len) {
    instr[pos + len] = '\0';
    return &instr[pos];
}

char *strFromLastChar(const char *str, const char ch) {
    size_t instrlen = strlen(str);
    char instr[instrlen];
    strcpy(instr, str); // preserve the original string
    char *chptr = strrchr(instr, ch);
    size_t pos = chptr - instr + 1; // skip ch
    size_t len = instrlen - pos - ((instr[instrlen - 2] == '"') ? 2 : 0); // if it ends with '"', remove it
    static char *ret; ret = (char*)malloc(instrlen);
    strcpy(ret, "\""); strcat(ret, substr(instr, pos, len)); strcat(ret, "\"");
    return ret;
}

char *setupTmpFile() {
    time_t tmpFileTime;
    time(&tmpFileTime);
    char tmpDir[STR_SZ];
    int tmpDirLen = GetTempPath(STR_SZ, tmpDir);
    if ((tmpDirLen == 0) || (tmpDirLen > STR_SZ)) printf("ERROR: could not determine the path of the %s dir", "%TEMP%");
    static char tmpFilePath[STR_SZ];
    sprintf_s(tmpFilePath, STR_SZ, "%syt-dlp_id_%lld", tmpDir, tmpFileTime);
    return tmpFilePath;
}

char *strEndAtNL(char *str) {
    for (size_t i = strlen(str); i > 0 ; i--) {
        if (str[i] == '\n') str[i] = '\0';
    }
    return str;
}

void printf_n(char c, int n) {
    if (n <= 0) return;
    for (int i = 0; i < n; i++) {
        printf("%c", c);
    }
}

char *readAndDeleteTmpFile(char *tmpFilePath) {
    FILE *f; f = fopen(tmpFilePath, "r");
    if (f == NULL) printf("ERROR: %s could not be opened for reading", tmpFilePath);
    static char videoFilePath[STR_SZ];
    if (fgets(videoFilePath, STR_SZ, f) == NULL) printf("ERROR: could not read from empty file");
    fclose(f);
    if (DeleteFileA(tmpFilePath) == 0) printf("ERROR: could not delete %s", tmpFilePath);
    size_t videoPathLen = strlen(videoFilePath);
    if (videoFilePath[videoPathLen] == '\0') return substr(videoFilePath, 0, videoPathLen - 1);
    return videoFilePath;
}

void processInputChar(char inputChar, bool delete, char *videoFilePath) {
    if (inputChar == 3) return; // 3 is ASCII for Ctrl-C
    if ((inputChar == 'd' || inputChar == ' ') && delete == false) {
       printf(GOUP GOTOX "X" GODOWN, 2);
       delete = true;
       inputChar = getch();
    }
    if (inputChar == 13) { // 13 is ASCII for Enter
        printf("\n");
        char vlcline[STR_SZ] = "vlc --play-and-exit ";
        strcat(vlcline, videoFilePath);
        runProcess(vlcline, delete);
        if (delete) { // still doesn't work, but now doesn't even throw an error
            char delline[STR_SZ] = "del ";
            strcat(delline, videoFilePath);
            runProcess(delline, true);
        }
    } else {
        // printf("ERROR: '%c' (%d) is not a valid option. Try again.\n", inputChar, inputChar);
        return processInputChar(getch(), delete, videoFilePath);
    }
}


#define YT_DLP_FORMAT          "bv[height=1080][format_note!*=Premium]+ba/best"
#define YT_DLP_OUTPUT_TYPE     "mp4"
#define YT_DLP_OUTPUT_TEMPLATE "%USERPROFILE%/Videos/yt-dlp_tmp/%(title)s.%(ext)s"
#define YT_DLP_SB_REMOVE       "sponsor"

void printOptions() {
    char optionsstr[] = "Options / Flags:";
    int optionsstrlen = strlen(optionsstr);
    char formatstr[]  = "  Download format / quality: " YT_DLP_FORMAT;
    int formatstrlen  = strlen(formatstr);
    char sblockstr[]  = "  Removing video segments: " YT_DLP_SB_REMOVE;
    int sblockstrlen  = strlen(sblockstr);
    char outputstr[]  = "  Output path: " YT_DLP_OUTPUT_TEMPLATE;
    int outputstrlen  = strlen(outputstr);
    
    int maxstrlen = max(max(max(optionsstrlen, formatstrlen), sblockstrlen), outputstrlen);
    
    printf("X-"); printf_n('-', maxstrlen);                                           printf("-X\n");
    printf("| "); printf("%s", optionsstr); printf_n(' ', maxstrlen - optionsstrlen); printf(" |\n");
    printf("| "); printf("%s", formatstr);  printf_n(' ', maxstrlen - formatstrlen);  printf(" |\n");
    printf("| "); printf("%s", sblockstr);  printf_n(' ', maxstrlen - sblockstrlen);  printf(" |\n");
    printf("| "); printf("%s", outputstr);  printf_n(' ', maxstrlen - outputstrlen);  printf(" |\n");
    // printf("| "); printf("  Output filetype: %s", YT_DLP_OUTPUT_TYPE "         "); printf(" |\n");
    printf("X-"); printf_n('-', maxstrlen);                                           printf("-X\n");
    printf("\n");
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("This program is intended to only be executed via the yt-dlp:// protocol.\n");
        printf("If this is actually the case, then there is a bug in here, somewhere ...\n");
        return 1;
    }
    
    if (strlen(argv[1]) <= 9) {
        printf("There probably is no video ID provided.\n");
        printf("Check the javascript code in the bookmark for errors.\n");
        return 2;
    }
    
    char *instr = argv[1]; // NOTE: doesn't create copy, just another pointer
    size_t pos = 9; // argv[1] = yt-dlp://R-DEp62qDeE/
    size_t instrlen = strlen(instr);
    size_t len = instrlen - pos - ((instr[instrlen - 1] == '/') ? 1 : 0); // if it ends with '/', remove it
    char *id = substr(instr, pos, len);
    
    char url[STR_SZ] = "https://www.youtube.com/watch?v=";
    strcat(url, id);
    
    char title[STR_SZ] = "JS calls YT-DLP - ID: ";
    strcat(title, id);
    SetConsoleTitle(title);
    
    // char *tmpFilePath = setupTmpFile();
    
    char cmdline[STR_SZ*2] = {0};
    strcat(cmdline, "yt-dlp -f \"" YT_DLP_FORMAT "\" --merge-output-format \"" YT_DLP_OUTPUT_TYPE "\" --embed-metadata -o \"" YT_DLP_OUTPUT_TEMPLATE "\" --sponsorblock-remove \"" YT_DLP_SB_REMOVE "\" -R infinite --no-playlist");
    
    // strcat(cmdline, " --download-sections *from-url"); // this produces some weird bug, but only when run from this code. the produced command on its own is fine.
    // sprintf(cmdline, "%s%s%s%s", cmdline, "--exec \"echo {} > ", tmpFilePath, "\""); // output video file name into random file
    strcat(cmdline, " \""); strcat(cmdline, url); strcat(cmdline, "\""); // append url
    
    printf("Running [%s]\n\n", cmdline);
    
    printOptions();
    
    int ret = runProcess(cmdline, true);
    
    if (ret == 0) printf("\nDone!\n");
    
    if (ret > 0) {
        printf("\n");
        printf(RED "ERROR: yt-dlp exited with code %d.\n" RESET, ret);
        printf("Something went wrong, see above printout from yt-dlp.\n");
        printf("In case you'd like to tweak the invoked command, have a shell:\n\n");
        runProcess("cmd", true);
    }
    
    // char *videoFilePath = readAndDeleteTmpFile(tmpFilePath);
    // char *videoFileName = strFromLastChar(videoFilePath, '\\');
    // // printf("\n");
    // // printf("The program will open %s in VLC as soon as you press Enter.\n", videoFileName);
    // // printf("[ ] " UNDERLINED BOLD "d" RESET "elete the file afterwards?\n");
    // // bool delete = false;
    // // char inputChar = getch();
    
    // // processInputChar(inputChar, delete, videoFilePath);
    
    // char vlcline[STR_SZ] = "start \"\" /min vlc --play-and-exit --start-paused ";
    // strcat(vlcline, videoFilePath);
    // runProcess(vlcline, false);

    return 0;
}