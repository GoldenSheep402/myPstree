#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// 进程节点结构
struct ProcessNode
{
    int pid;                     // 进程ID
    char name[256];              // 进程名
    struct ProcessNode *parent;  // 父进程指针
    struct ProcessNode *child;   // 子进程指针
    struct ProcessNode *sibling; // 兄弟进程指针
};

struct ProcessNode *new_node();
void read_pid(const char *pid_dir, struct ProcessNode *pid_node);
void add_child_process(struct ProcessNode *node, struct ProcessNode *child, int ppid);
struct ProcessNode *new_node();
void print_indent(int level, int is_last_sibling);
void traverse_process_tree(struct ProcessNode *node, char *prefix);

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "-a") == 0)
    {
        struct ProcessNode *dummy_head = new_node();
        struct ProcessNode *root_node = new_node();
        strcpy(root_node->name, "root");
        root_node->pid = 0;

        dummy_head->child = root_node;
        root_node->parent = dummy_head;

        assert(!argv[argc]);
        read_pid("/proc", root_node);

        printf(argv[1]);
        printf("Print All Process\n");
        char prefix[256] = "";
        traverse_process_tree(dummy_head->child, prefix);
    }
    else
    {
        printf("没有提供命令行参数 -a。\n");
    }

    return 0;
}

void read_pid(const char *pid_dir, struct ProcessNode *dummy_head)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir(pid_dir);
    if (dir == NULL)
    {
        perror("opendir");
        return;
    }

    // 读取目录
    while ((entry = readdir(dir)) != NULL)
    {
        // 检查是否为数字的目录项（即PID目录）
        if (entry->d_type == DT_DIR && atoi(entry->d_name) != 0)
        {
            char proc_path[256];
            FILE *status_file;
            char line[256];
            char name[256] = "";
            char ppid[256] = "";

            // 构建/proc/<pid>/status文件路径
            snprintf(proc_path, sizeof(proc_path), "%s/%s/status", pid_dir, entry->d_name);
            status_file = fopen(proc_path, "r");
            if (status_file == NULL)
            {
                perror("fopen");
                continue;
            }

            struct ProcessNode *next_pid_node = new_node();

            next_pid_node->pid = atoi(entry->d_name);

            // 在status文件中读取相关信息
            while (fgets(line, sizeof(line), status_file) != NULL)
            {
                if (strncmp(line, "Name:", 5) == 0)
                {
                    size_t len = strcspn(line + 6, "\n");
                    strncpy(name, line + 6, len);
                    name[len] = '\0'; // 手动添加字符串结束符
                    strcpy(next_pid_node->name, name);
                }
                if (strncmp(line, "PPid:", 5) == 0)
                {

                    size_t len = strcspn(line + 6, "\n");
                    strncpy(ppid, line + 6, len);
                    ppid[len] = '\0'; // 手动添加字符串结束符

                    break;
                }
            }
            fclose(status_file);
            // add child process
            add_child_process(dummy_head, next_pid_node, atoi(ppid));
        }
    }

    closedir(dir);
}

struct ProcessNode *new_node()
{
    struct ProcessNode *pid_node = malloc(sizeof(struct ProcessNode));
    if (pid_node == NULL)
    {
        // 处理内存分配失败的情况
        perror("Memory allocation failed");
        return NULL;
    }

    memset(pid_node, 0, sizeof(struct ProcessNode));
    return pid_node;
}

// 将子进程添加到父进程
void add_child_process(struct ProcessNode *node, struct ProcessNode *child, int ppid)
{
    if (node == NULL)
    {
        return;
    }

    if (node->pid == ppid)
    {
        if (node->child == NULL)
        {
            node->child = child;
        }
        else
        {
            struct ProcessNode *temp = node->child;
            while (temp->sibling != NULL)
            {
                temp = temp->sibling;
            }
            temp->sibling = child;
        }
        child->parent = node;
        return;
    }

    // add node
    add_child_process(node->child, child, ppid);
    add_child_process(node->sibling, child, ppid);

    return;
}

// tree struct
void print_indent(int level, int is_last_sibling)
{
    for (int i = 1; i < level; i++)
    {
        printf("│  ");
    }
    if (level > 0)
    {
        if (is_last_sibling)
        {
            printf("└─");
        }
        else
        {
            printf("├─");
        }
    }
}

// DFS
void traverse_process_tree(struct ProcessNode *node, char *prefix)
{
    if (node == NULL)
    {
        return;
    }

    // Print prefix
    printf("%s", prefix);

    // judge is last sibling
    int is_last_sibling = (node->sibling == NULL);
    if (is_last_sibling)
    {
        printf("└─PID: %d, Name: [%s]\n", node->pid, node->name);
    }
    else
    {
        printf("├─PID: %d, Name: [%s]\n", node->pid, node->name);
    }

    // Create new prefix for children
    size_t prefix_length = 255;
    char *child_prefix = malloc(255);
    strcpy(child_prefix, prefix);
    if (is_last_sibling)
    {
        strcat(child_prefix, "    "); // last sibling
    }
    else
    {
        strcat(child_prefix, "│  "); // not last sibling
    }

    traverse_process_tree(node->child, child_prefix);
    free(child_prefix); // free the allocated memory!

    traverse_process_tree(node->sibling, prefix);
}
