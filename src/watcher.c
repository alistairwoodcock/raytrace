// Watchman Replacement because watchman is now breaking and we shouldn't
// depend on shit that can break so easily

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#include "shared/utils.h"
#include "shared/array.h"


struct TrackedFile {
    char* path;
    time_t mod_time;
};

struct TrackedFile** tracked_files;

struct TrackedFile* get_tracked(char* path){
    for(int i = 0; i < a_count(tracked_files); i++){
        struct TrackedFile* tracked_file = tracked_files[i];

        if(tracked_file == null) continue;

        bool same = true;

        char* tracked_path = tracked_file->path;

        if(a_count(tracked_path) != a_count(path)) continue;
        
        for(int j = 0; j < a_count(tracked_path); j++){
            if(tracked_path[j] != path[j]){
                same = false;
                break;
            }
        }
    
        if(same) return tracked_file;
    }

    return null;
}

void track(char* path){
    char* new_tracked_file_path = a_new(char, 20);


    for(int i = 0; i < a_count(path); i++) a_push(new_tracked_file_path, path[i]);

    printf("tracking new file: %s\n", new_tracked_file_path);

    struct stat attr;
    stat(new_tracked_file_path, &attr);

    struct TrackedFile* file = (struct TrackedFile*)malloc(sizeof(struct TrackedFile));
    file->path = new_tracked_file_path;
    file->mod_time = attr.st_mtime;

    a_push(tracked_files, file);
}

bool check_modified(struct TrackedFile* file){
    struct stat attr;

    if(stat(file->path, &attr) == 0 && file->mod_time != attr.st_mtime){
        printf("%s modified\n", file->path);
        file->mod_time = attr.st_mtime;
        return true;
    }

    return false;
}



bool recurse_dir(char* dirname_part, int count){

    bool files_changed = false;

    //printf("%s\n", dirname_part);

    struct dirent *de;

    // copy name into new memory
    char* full_path = a_new(char, 257);
    for(int i = 0; i < a_count(dirname_part); i++) a_push(full_path, dirname_part[i]);
    a_push(full_path, '/');


    DIR *dir = opendir(full_path);

    if(dir == null){
        printf("couldn't open directory.\n");
        goto cleanup;
    }

    // get all files in src dir that have .h or .c
    // store files and their change date or hash
    // run 'compile' script when files changes
    // print stdout of compile script

    while ((de = readdir(dir)) != NULL){
        
        char* new_path = a_new(char, 256);
        int len, i = 0;
        while(de->d_name[i++] != '\0' && i < 257);
        len = i - 1;

        for(int i = 0; i < a_count(full_path); i++) a_push(new_path, full_path[i]);
        for(int i = 0; i < len; i++) a_push(new_path,de->d_name[i]);


        if(de->d_type == 4 && de->d_name[0] != '.') 
        {
            files_changed = recurse_dir(new_path, count+1) || files_changed;
        } 
        else 
        {
            if(de->d_name[len-2] != '.') continue;
            if(de->d_name[len-1] != 'c' && 
               de->d_name[len-1] != 'h') continue;

            struct TrackedFile* file = get_tracked(new_path);

            if(file == null){
                track(new_path);
                files_changed = true;
            } else {
                files_changed = check_modified(file) || files_changed;
            }
        }

        a_free(new_path);
    }


cleanup:
    a_free(full_path);
    closedir(dir);

    return files_changed;
}

bool check_tracked_files(){
    for(int i = 0; i < a_count(tracked_files); i++){
        struct stat attr;
        struct TrackedFile* file = tracked_files[i];

        if(file == null) continue;
        
        bool err = stat(file->path, &attr);
        if(err){
            printf("%s removed\n", file->path);
            a_free(file->path);
            free(file);
            
            tracked_files[i] = null;
            
            return true;
        }
    }

    return false;
}

bool check_for_changes(){

    bool changed = check_tracked_files();

    if(!changed){
        char* file = a_new(char, 256);
        a_push(file, '.');

        changed = recurse_dir(file, 0);
    } 

    return changed;
}

int main(void){
    
    tracked_files = a_new(struct TrackedFile*, 40);

    while(true){
        
        bool changed = check_for_changes();
        
        if(changed){
            printf("gotta run that recompile\n\n");
            system("./compile");
        }

        usleep(400000);
    }

    return 0;
}
