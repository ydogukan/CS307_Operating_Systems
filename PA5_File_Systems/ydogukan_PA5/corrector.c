#include <stdio.h>
#include <string.h>
#include <dirent.h>

typedef struct Person {
    char name[30], surname[30];
    char gender;
} Person;

const char* SSCANF_FORMAT = "%c %s %s\n";

int modifyPerson(Person p, FILE* in) {
    char word[100];

    fscanf(in, "%s", word);
    fseek(in, -strlen(word), SEEK_CUR);

    if ((p.gender == 'f') && (strcmp(word, "Ms.") != 0)) {
        fputs("Ms.", in);
    }

    else if ((p.gender == 'm') && (strcmp(word, "Mr.") != 0)) {
        fputs("Mr.", in);
    }

    else {
        fseek(in, 3, SEEK_CUR);
    }

    fscanf(in, "%s", word);
    fscanf(in, "%s", word);
    //printf("%s => %ld\n", word, ftell(in) - strlen(word));
    fseek(in, -strlen(word), SEEK_CUR);

    if (strcmp(word, p.surname) != 0) {
        fputs(p.surname, in);
    }
    
    else {
        fseek(in, strlen(p.surname), SEEK_CUR);
    }

    return 0;
}

int readDatabaseAndModifyPerson(char* word, FILE* db, FILE* in) {
    char line[100];
    while (fgets(line, sizeof(line), db)) {
        Person p;
        int elements_read = sscanf(line, SSCANF_FORMAT, &p.gender, &p.name, &p.surname);
        
        if (elements_read < 3) {
            printf("Less than 3 elements in a database row, please make sure database.txt is formatted correctly.\n");
            return 1;
        }

        if (strcmp(p.name, word) == 0) {
            return modifyPerson(p, in);
        }
    }

    return 2;
}

int readAndModifySingleTextFile(char* filename, FILE* db) {
    FILE* in = fopen(filename, "r+");
    if (in == NULL) {
        printf("%s could not be opened.", filename);
        return 1;
    }

    char word[100];
    int foundPerson = 0;
    while (fscanf(in, "%s", word) != EOF) {
        //printf("%s => %ld\n", word, ftell(in) - strlen(word));

        if ((strcmp(word, "Ms.") == 0) || (strcmp(word, "Mr.") == 0)) {
            foundPerson = 1;
        }

        else if (foundPerson == 1){
            foundPerson = 0;
            fseek(in, - 4 - strlen(word), SEEK_CUR);
            readDatabaseAndModifyPerson(word, db, in);
            fseek(db, 0, SEEK_SET);
        }
    }

    fclose(in);
    return 0;
}

void readAndModifyAllTextFiles(const char* dirname, FILE* db) {
    DIR* dir = opendir(dirname);
    if (dir == NULL) {
        return;
    }

    struct dirent* direntPtr;
    direntPtr = readdir(dir);

    while (direntPtr != NULL) {
        if (strcmp(direntPtr->d_name, ".") != 0 && strcmp(direntPtr->d_name, "..") != 0) {
            char path[200] = {0};
            strcat(path, dirname);
            strcat(path , "/");
            strcat(path, direntPtr->d_name);
            //printf("%s\n", path);

            if (direntPtr->d_type == DT_DIR) {
                readAndModifyAllTextFiles(path, db);
            }

            else if (direntPtr->d_type == DT_REG) {
                char* ext = strrchr(direntPtr->d_name, '.');
                if (ext != NULL) {
                    if (strcmp(ext, ".txt") == 0) {
                        if (strcmp(dirname, ".") != 0) {
                            readAndModifySingleTextFile(path, db);
                        }
                        
                        else {
                            if (strcmp(direntPtr->d_name, "database.txt") != 0) {
                                readAndModifySingleTextFile(path, db);
                            }
                        }
                    }
                }
            }
        }

        direntPtr = readdir(dir);
    }

    closedir(dir);
}

int main(int argc, char* argv[]) {
    FILE* db = fopen("database.txt", "r");

    if (db == NULL) {
        printf("database.txt could not be opened.\n");
        return 1;
    }
    
    readAndModifyAllTextFiles(".", db);

    fclose(db);
    return 0;
}