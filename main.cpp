#include <iostream>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <vector>
#include <dirent.h>


using namespace std;

struct FileData {
    string fileName;
    string fileSourceFolder;
    string fileRecieveFolder;
    stat fileStat;
};

struct CopyArgs {
    vector<FileData> *files;
    int numFinished;
    pthread_mutex_t mutex;
};

void *copy(void *arg) {
    CopyArgs *copyArgs = (CopyArgs *) arg;
    pthread_mutex_lock(&copyArgs->mutex);
    string fileName = copyArgs->files->at(copyArgs->numFinished).fileName;
    string fileSourceFolder = copyArgs->files->at(copyArgs->numFinished).fileSourceFolder;
    string fileRecieveFolder = copyArgs->files->at(copyArgs->numFinished).fileRecieveFolder;
    pthread_mutex_unlock(&copyArgs->mutex);
    int sourceFd = open((char *) (string(fileSourceFolder) + "/" + string(fileName)).c_str(), O_RDONLY);
    int reciveFd;
    if ((reciveFd = open((char *) (string(fileRecieveFolder) + "/" + string(fileName)).c_str(), O_RDONLY)) >= 0)
    {
        int recieveFdOld=open((char *) (string(fileRecieveFolder) + "/" + string(fileName)+".old").c_str(),O_WRONLY);
    }

}

int funccmp(const void *val1, const void *val2) {
    return (int) (((FileData *) val2)->fileStat.st_size - ((FileData *) val1)->fileStat.st_size);
}

int main(int argc, char **argv) {
    int threadCount = atoi(argv[1]);
    char *sourceFolder = argv[2];
    char *recieveFolder = argv[3];

    vector<FileData> files;
    struct dirent *fileInFolder;
    DIR *d = opendir(sourceFolder);

    while ((fileInFolder = readdir(d)) != NULL) {
        if (fileInFolder->d_type == DT_REG) {
            FILE *fp;
            stat buff;
            /*char fileName[strlen(sourceFolder) + 1 + strlen(fileInFolder->d_name)];
            for (int i = 0; i < strlen(sourceFolder); i++) {
                fileName[i] = sourceFolder[i];
            }
            fileName[strlen(sourceFolder)] = '/';
            for (int i = 0; i < strlen(fileInFolder->d_name); i++) {
                fileName[i + 1 + strlen(sourceFolder)] = fileInFolder->d_name[i];
            }*/
            char *fileName = (char *) (string(sourceFolder) + "/" + string(fileInFolder->d_name)).c_str();
            if ((fp = fopen(fileName, "rb")) == NULL) {
                printf("Cannot open file %s.\n", fileInFolder->d_name);
            }
            else {
                fstat(fileno(fp), &buff);
                if (buff.st_size > 0) {
                    FileData fileData;
                    fileData.fileName = string(fileInFolder->d_name);
                    fileData.fileSourceFolder = string(sourceFolder);
                    fileData.fileRecieveFolder = string(recieveFolder);
                    fileData.fileStat = buff;
                    files.push_back(fileData);
                }
                fclose(fp);
            }
        }
    }
    closedir(d);
    qsort(&files[0], files.size(), sizeof(FileData), funccmp);

    pthread_t threads[threadCount];
    CopyArgs copyArgs;
    pthread_mutex_init(&copyArgs.mutex, NULL);
    copyArgs.files = &files;
    copyArgs.numFinished = 0;
    for (int i = 0; i < threadCount; i++) {
        pthread_create(&threads[i], NULL, copy, &copyArgs);
    }
    return 0;
}