// FLX/DAT Extractor

#include <windows.h>
#include <RestartManager.h>
#include <winternl.h>
#include <stdio.h>
#include <iostream>
#include <vector>

using namespace std;

struct flx_header_s
{
    char unknown[80];
    long unknown2;
    long numOfFiles;
    long unknown3;
    long fileSize;
    long entrySize;
    long unknown4;
};

struct flx_res_s
{
    unsigned int offset;
    int size;
};

typedef unsigned short uint16;
typedef unsigned long uint32;

void CreateDir(const char *name)
{
    CreateDirectoryA(name, nullptr);
}

void GetAbsolutePath(char *path)
{
    char buf[MAX_PATH];
    GetModuleFileNameA(NULL, buf, MAX_PATH);
    strcat(path, buf);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s <FLX file>\n", argv[0]);
        return -1;
    }

    FILE *flxFile = fopen(argv[1], "rb");
    if (!flxFile) return -1;

    char folderPath[MAX_PATH];
    strcpy(folderPath, argv[1]);

    if (char *c = strrchr(folderPath, '.'))
        *c = '\0';

    CreateDir(folderPath);

    flx_header_s fileInfo;

    fread(&fileInfo.unknown, sizeof(fileInfo.unknown), 1, flxFile);
    fread(&fileInfo.unknown2, sizeof(fileInfo.unknown2), 1, flxFile);
    fread(&fileInfo.numOfFiles, sizeof(fileInfo.numOfFiles), 1, flxFile);
    fread(&fileInfo.unknown3, sizeof(fileInfo.unknown3), 1, flxFile);
    fread(&fileInfo.fileSize, sizeof(fileInfo.fileSize), 1, flxFile);
    fread(&fileInfo.unknown2, sizeof(fileInfo.unknown2), 1, flxFile);

    printf("Number of files: %d\n", fileInfo.numOfFiles);
    printf("----------------------------------\n");

    fseek(flxFile, 0x80, SEEK_SET);

    vector<flx_res_s> resources;

    for (int i = 0; i < fileInfo.numOfFiles; i++)
    {
        unsigned long offset;
        fread(&offset, sizeof(offset), 1, flxFile);

        if (offset)
        {
            long size;
            char buf[64];
            buf[0] = '\0';

            fread(&size, sizeof(size), 1, flxFile);
            resources.push_back({ offset, size });
        }
    }

    for (int i = 0; i < resources.size(); i++)
    {
        printf("File: %d\n", i);
        printf("Offset: %d\n", resources[i].offset);
        printf("Size: %d\n", resources[i].size);
        printf("------------\n");
    }

    for (int i = 0; i < resources.size(); i++)
    {
        char resPath[256];
        strcpy(resPath, folderPath);
        if (resPath[strlen(resPath) - 1] != '/') strcat(resPath, "/");
        char indexStr[8];
        sprintf(indexStr, "_%d", i);
        strcat(resPath, argv[1]);
        strcat(resPath, indexStr);

        FILE *resFile = fopen(resPath, "wb");
        if (!resFile) return -2;

        fseek(flxFile, resources[i].offset, SEEK_SET);

        for (int j = 0; j < resources[i].size; j++)
        {
            char c;
            fread(&c, sizeof(char), 1, flxFile);
            fwrite(&c, sizeof(char), 1, resFile);
        }

        fclose(resFile);
    }

    fclose(flxFile);

    return 0;
}
