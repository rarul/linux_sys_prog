#include  <stdio.h>
#include  <string.h>
#include  <dirent.h>

static void walk_reg(char *path) {
    puts(path);
}
static int is_dir_visit(char *name) {
    if (strcmp(name, ".")==0) {
        return 0;
    } else if (strcmp(name, "..")==0) {
        return 0;
    }
    return 1;
}
static void walk_dir(char *path, int path_length) {
    struct dirent *dent;
    DIR *dir;

    dir = opendir(path);
    if (dir == NULL) {
        perror(path);
        return;
    }
    while ((dent = readdir(dir))) {
        switch (dent->d_type) {
        case DT_REG:
            strcpy (&path[path_length], dent->d_name);
            walk_reg(path);
            break;
        case DT_DIR:
            if (is_dir_visit(dent->d_name)) {
                int d_name_len = strlen(dent->d_name);
                strcpy (&path[path_length], dent->d_name);
                strcpy (&path[path_length+d_name_len], "/");
                walk_dir (path, path_length + d_name_len + 1);
            }
            break;
        case DT_CHR:
        case DT_BLK:
        case DT_FIFO:
        case DT_SOCK:
        case DT_LNK:
        default:
            break;
        }
    }
    closedir(dir);
    return;
}

int main(int argc, char *argv[]){
    char path[PATH_MAX];
    strcpy (path, "./");
    if (argc >= 2) {
        strcpy (path, argv[1]);
        if (path[strlen(path)-1] != '/') {
            strcat (path, "/");
        }
    }

    walk_dir(path, strlen(path));
    return 0;
}
