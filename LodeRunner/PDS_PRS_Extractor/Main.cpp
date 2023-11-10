// PDS/PRS pack extractor

#include <windows.h>
#include <RestartManager.h>
#include <winternl.h>
#include <stdio.h>
#include <iostream>
#include <vector>

using namespace std;

typedef unsigned short uint16;
typedef unsigned long uint32;

struct prd_header_s
{
    uint16 unknown;
    char filename[256];
    uint32 dummyFileEntry;
    uint16 unknown2;
    uint16 unknown3;
    uint16 unknown4;
    uint16 numOfFiles;
    char unknown5[44];
};

struct prd_res_s
{
    uint16 fileID;
    uint16 unknown;
    uint16 unknown2;
    uint32 unknown3;
    uint32 fileOffset;
    char fileType_Extension[4];
    uint16 unknown4;
    char filename[18];
    uint32 fileSize;
};

void CreateDir(const char *path)
{
    char buf[MAX_PATH];
    strcpy(buf, path);

    if (char *c = strrchr(buf, '.'))
        *c = '\0';

    CreateDirectoryA(buf, nullptr);
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
        printf("Usage: %s <PRD file>\n", argv[0]);
        return -1;
    }

    FILE *prdFile = fopen(argv[1], "rb");
    if (!prdFile) return -1;

    prd_header_s prdHeader;
    fread(&prdHeader.unknown, sizeof(uint16), 1, prdFile);
    fread(&prdHeader.filename, sizeof(prdHeader.filename), 1, prdFile);
    fread(&prdHeader.dummyFileEntry, sizeof(uint32), 1, prdFile);
    fread(&prdHeader.unknown2, sizeof(uint16), 1, prdFile);
    fread(&prdHeader.unknown3, sizeof(uint16), 1, prdFile);
    fread(&prdHeader.unknown4, sizeof(uint16), 1, prdFile);
    fread(&prdHeader.numOfFiles, sizeof(uint16), 1, prdFile);
    fread(&prdHeader.unknown5, sizeof(prdHeader.unknown5), 1, prdFile);

    printf("File data path: %s\n", prdHeader.filename);
    printf("Number of files: %d\n", prdHeader.numOfFiles);
    printf("----------------------------------\n");

    char prsPath[128];
    strcpy(prsPath, argv[1]);
    prsPath[strlen(prsPath) - 1] += 15; // Changes extension from prd to prs

    char folderPath[MAX_PATH];
    strcpy(folderPath, prsPath);

    if (char *c = strrchr(folderPath, '.'))
        *c = '\0';

    CreateDir(folderPath);

    FILE *prsFile = fopen(prsPath, "rb");
    if (!prsFile) return -1;

    for (int i = 0; i < prdHeader.numOfFiles; i++)
    {
        prd_res_s fileInfo;

        memset(&fileInfo, 0, sizeof(prd_res_s));
        fread(&fileInfo.fileID, sizeof(uint16), 1, prdFile);
        fread(&fileInfo.unknown, sizeof(uint16), 1, prdFile);
        fread(&fileInfo.unknown2, sizeof(uint16), 1, prdFile);
        fread(&fileInfo.unknown3, sizeof(uint32), 1, prdFile);
        fread(&fileInfo.fileOffset, sizeof(uint32), 1, prdFile);
        fread(&fileInfo.fileType_Extension, sizeof(fileInfo.fileType_Extension), 1, prdFile);
        fread(&fileInfo.unknown4, sizeof(uint16), 1, prdFile);
        fread(&fileInfo.filename, sizeof(fileInfo.filename), 1, prdFile);
        fread(&fileInfo.fileSize, sizeof(uint32), 1, prdFile);

        printf("File ID: %d\n", fileInfo.fileID);
        printf("Filename: %s\n", fileInfo.filename);
        printf("File format: %s\n", fileInfo.fileType_Extension);
        printf("File Size: %d\n", fileInfo.fileSize);
        printf("File Offset: %d\n", fileInfo.fileOffset);
        printf("\n");

        fseek(prsFile, fileInfo.fileOffset, SEEK_SET);

        char newfln[256];
        strcpy(newfln, folderPath);
        if (newfln[strlen(newfln) - 1] != '/') strcat(newfln, "/");
        strcat(newfln, fileInfo.filename);
        strcat(newfln, ".");
        strcat(newfln, fileInfo.fileType_Extension);

        FILE *res = fopen(newfln, "wb");
        if (!res) return -2;

        for (int j = 0; j < int(fileInfo.fileSize); j++)
        {
            char c;
            fread(&c, sizeof(char), 1, prsFile);
            fwrite(&c, sizeof(char), 1, res);
        }

        fclose(res);
    }

    fclose(prsFile);
    fclose(prdFile);

    return 0;
}
