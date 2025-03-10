#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>

#define PROC_DIR "/proc"
#define STAT_FILE "stat"
#define CMDLINE_FILE "cmdline"
#define COMM_FILE "comm"
#define MAX_LINE 1024

// 从 /proc/<PID>/stat 文件中解析进程状态
char get_process_state(const char *pid) {
    char path[MAX_LINE];
    FILE *fp;
    char state = '?';
    char line[MAX_LINE];

    // 构造 /proc/<PID>/stat 文件路径
    snprintf(path, sizeof(path), "%s/%s/%s", PROC_DIR, pid, STAT_FILE);

    fp = fopen(path, "r");
    if (fp == NULL) {
        return state;
    }

    // 读取 stat 文件内容
    if (fgets(line, sizeof(line), fp) != NULL) {
        // TODO: 解析进程状态（第 3 个字段）
        int cnt;
        int p = 0;
        for (cnt = 0; cnt < 2; ++cnt) {
            while (line[p] != ' ') ++p;
            while (line[p] == ' ') ++p;    // skip the space
        }
        state = line[p];
    }

    fclose(fp);
    return state;
}

// 从 /proc/<PID>/cmdline 文件中获取命令行参数
void get_process_cmdline(const char *pid, char *cmdline, size_t size) {
    char path[MAX_LINE];
    FILE *fp;

    // 构造 /proc/<PID>/cmdline 文件路径
    snprintf(path, sizeof(path), "%s/%s/%s", PROC_DIR, pid, CMDLINE_FILE);


	// char *ret = NULL;
    int read = 0;
    fp = fopen(path, "r");
    if (fp != NULL) {
        // TODO: 读取 cmdline 文件内容
        read = fread(cmdline, 1, size, fp);
        // Note that ^@ in `/proc/<pid>/cmdline` is '\0'
        // We use fread to copy the binary into array
        // Then we need to manually replace '\0' with ' '
        // Otherwise, the output will be truncated.

        int i;
        for (i = 0; i < read; ++i) {
            if (cmdline[i] == '\0') cmdline[i] = ' ';
        }
        cmdline[read] = '\0';
        fclose(fp);
    }

    // 如果 cmdline 为空，则读取 comm 文件
    if (read == 0) {
        // 构造 /proc/<PID>/comm 文件路径
        snprintf(path, sizeof(path), "%s/%s/%s", PROC_DIR, pid, COMM_FILE);

        // TODO: 读取 comm 文件内容

        fp = fopen(path, "r");
        if (fp != NULL) {
            read = fread(cmdline, 1, size, fp);
            int i;
            for (i = 0; i < read; ++i) {
                if (cmdline[i] == '\0') cmdline[i] = ' ';
            }
            cmdline[read] = '\0';
            fclose(fp);
        } else {
            snprintf(cmdline, size, "[unknown]");
        }
    }
}

int main() {
    DIR *dir;
    struct dirent *entry;

    // 打开 /proc 目录
    dir = opendir(PROC_DIR);
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    // 遍历 /proc 目录下的所有文件和目录
    while ((entry = readdir(dir)) != NULL) {
        // 只处理 PID 目录（目录名是数字）
        if (isdigit(entry->d_name[0])) {
            char path[MAX_LINE];
            struct stat statbuf;

            // 构造 /proc/<PID> 路径
            snprintf(path, sizeof(path), "%s/%s", PROC_DIR, entry->d_name);

            // 使用 stat 检查是否是目录
            if (stat(path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
                char pid[16] = "";
                char state = '?';
                char cmdline[MAX_LINE] = "";

                // TODO: 获取 PID
                strncpy(pid, entry->d_name, sizeof(pid) - 1);
                pid[15] = '\0';
                // TODO: 获取进程状态
                state = get_process_state(pid);
                // TODO: 获取命令行参数
                get_process_cmdline(pid, cmdline, sizeof(cmdline));
                // TODO: 输出 PID（5 字符宽度，右对齐），状态，命令行参数
                printf("%5s %c %s\n", pid, state, cmdline);
            }
        }
    }

    closedir(dir);
    return 0;
}
