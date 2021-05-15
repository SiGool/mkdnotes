/*
Copyright (c) 2018 石固

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef _WIN32
 #include <wchar.h>
#endif
#include "./vendor/whereami/whereami.c"
#include "./notesConfReader.c"
#include "./Indexes2JSON.c"

void printHelp(void);
void printVersion(void);
void recreateData(void);
void createTemplate(char namePart[], int useConf);
char* getBaseDir(void);
#ifdef _WIN32
 char* getAsc2u8s(char *src);
#endif

#define lastChar(str) str[strlen(str) - 1]

const char DIRECTORY_SEPARATOR[] = 
    #ifdef _WIN32
        "\\"
    #else
        "/"
    #endif
;

const int MAJOR_VERSION_NUMBER = 1;
const int MINOR_VERSION_NUMBER = 0;
const int BUILD_VERSION_NUMBER = 0;

int main(int argc, char *argv[])
{
    if (argc == 1)
        printHelp();
    else if (argc == 2)
    {
        if (strcmp(argv[1], "--help") == 0)
            printHelp();
        else if (strcmp(argv[1], "--version") == 0)
            printVersion();
        else if (strcmp(argv[1], "--commit") == 0)
            recreateData();
        else
        {
            fprintf(stderr, "error: Invalid argument\n");
            exit(1);
        }
    }
    else if ((argc == 3 || argc == 4) && strcmp(argv[1], "--init") == 0 && abs(strcmp(argv[2], "-o") != 0) + argc == 4)
        createTemplate(argv[argc - 1], argc - 3);
    else
    {
        fprintf(stderr, "error: Invalid argument\n");
        exit(1);
    }

    return 0;
}

void printHelp(void)
{
    printf("usage:\n\n");
    printf("  --help                  Display this information.\n");
    printf("  --version               Display mkdnotes version information.\n");
    printf("  --init [-o] filename    Create a markdown note template file in current directory.\n");
    printf("                          if you add \"-o\" option, the configuration file will be read.\n");
    printf("  --commit                Regenerate data.js file base on the current files and folders structure.\n");
}

void printVersion(void) 
{
    printf("mkdnotes version %d.%d.%d\n", MAJOR_VERSION_NUMBER, MINOR_VERSION_NUMBER, BUILD_VERSION_NUMBER);
    printf("Copyright (C) 2018 SiGool\n");
}

void recreateData(void) 
{
    char *baseDir = getBaseDir();

    char dataJsPath[strlen(baseDir) + 20];
    memset(dataJsPath, '\0', strlen(dataJsPath));
    strcpy(dataJsPath, baseDir);
    strcat(dataJsPath, "/assets/data.js");

    FILE *fp = NULL;
    if ((fp = fopen(dataJsPath, "w")) == NULL)
    {
        fprintf(stderr, "error: Failed to create data file: %s\n", dataJsPath);
        exit(1);
    }

    char notesPath[strlen(baseDir + 10)];
    memset(notesPath, '\0', strlen(notesPath));
    strcpy(notesPath, baseDir);
    strcat(notesPath, "/notes");

      free(baseDir);  

    char *res = Idx2JSON_printStr(notesPath);

    char dataStr[strlen(res) + 20];
    memset(dataStr, '\0', strlen(dataStr));
    strcat(dataStr, "var data = ");
    strcat(dataStr, res);
    strcat(dataStr, ";");

      cJSON_free(res); 

    /*
     * Only support: Windows(Ascii), linux/mac(utf8)
    */
    #ifdef _WIN32

        char *u8s;
        if((u8s = getAsc2u8s(dataStr)) == NULL)
            fprintf(stderr, "error: Failed to create data file: %s\n", dataJsPath);
        else
        {
            fputs(u8s, fp);
            free(u8s);
        }
    #else
        fputs(dataStr, fp);
    #endif

    fclose(fp);
}

void createTemplate(char namePart[], int useConf)
{
    // get current working directory 
    char *curWorkingDir;
    if ((curWorkingDir = getcwd(NULL, 0)) == NULL) 
    {
        fprintf(stderr, "error: Failed to get current working directory\n");
        exit(1);
    }
    
    if (lastChar(curWorkingDir) == DIRECTORY_SEPARATOR[0])
        lastChar(curWorkingDir) = '\0';

    // get basedir path
    char *baseDir = getBaseDir();

    // check working directory's location
    char testPath[strlen(baseDir) + 20];
    memset(testPath, '\0', strlen(testPath));

    strcpy(testPath, baseDir);
    strcat(testPath, DIRECTORY_SEPARATOR);
    strcat(testPath, "notes");
    
    free(baseDir);

    if (strcmp(curWorkingDir, testPath) != 0)
    {
        // match: .+(\\|\/)notes(\\|\/)(.+)?
        strcat(testPath, DIRECTORY_SEPARATOR);

        if (memcmp(curWorkingDir, testPath, strlen(testPath)) != 0)
        {
            lastChar(testPath) = '\0';
            fprintf(stderr, "error: Out of directory: %s\n", testPath);
            exit(1);    
        }
        else
        {
            lastChar(testPath) = '\0';
        }
    }


    // calculate configuration values
    configuration conf;

    if (useConf)
    {
        testPath[strlen(testPath) - strlen("notes")] = '\0';
        strcat(testPath, "mkdnotes.ini");
        conf = getConfOptions(testPath);

        testPath[strlen(testPath) - strlen("mkdnotes.ini") - 1] = '\0';
    }
    else
    {
        // default
        conf.title = "My Markdown Notes";
        conf.link_icon = "Yes";
        conf.theme = "simplex";

        testPath[strlen(testPath) - strlen("notes") - 1] = '\0';
    }


    // create template file
    char rPath[50] = {'\0'};
    int sIdx = strlen(testPath);

    while (curWorkingDir[sIdx] != '\0')
    {
        if (curWorkingDir[sIdx] == DIRECTORY_SEPARATOR[0])
            strcat(rPath, "../");
        sIdx++;
    }

    free(curWorkingDir);


    // create 
    char linkIcon[120] = {'\0'};

    if (strcmp(conf.link_icon, "Yes") == 0)
        sprintf(linkIcon, "<link rel=\"shortcut icon\" href=\"%sfavicon.ico\">\n", rPath);

    char templateStr[] = "<!DOCTYPE html>\n"
                         "<html>\n"
                         "<title>%s</title>\n"
                         "%s\n"

                         "<xmp theme=\"%s\" style=\"display:none;\">\n\n"

                         "</xmp>\n\n"

                         "<script src=\"%sassets/strapdown/strapdown.js\"></script>\n"
                         "</html>";

    char fileName[strlen(namePart) + 10];
    memset(fileName, '\0', strlen(fileName));

    strcpy(fileName, namePart);
    strcat(fileName, ".html");
    
    FILE *fp = NULL;
    if ((fp = fopen(fileName, "w+")) == NULL)
    {
        fprintf(stderr, "Failed to create template file\n");
        exit(1);
    }
    
    fprintf(fp, templateStr, conf.title, linkIcon, conf.theme, rPath);
    fclose(fp);
}

/*
 *****************
     Helper
 ****************
*/

char* getBaseDir(void)
{
    char *baseDir = NULL;
    int length, dirname_length;

    length = wai_getExecutablePath(NULL, 0, &dirname_length);
    if (length > 0)
    {
        baseDir = (char*)malloc(length + 1);
        if (!baseDir)
        {
            fprintf(stderr, "error: Failed to get the root directory\n");
            exit(1);
        }
        
        wai_getExecutablePath(baseDir, length, &dirname_length);
        baseDir[dirname_length] = '\0';
    }
    else 
    {

        fprintf(stderr, "error: Failed to get the root directory\n");
        exit(1);
    }

    if (lastChar(baseDir) == DIRECTORY_SEPARATOR[0])
        lastChar(baseDir) = '\0';

    return baseDir;
}

#ifdef _WIN32
 char* getAsc2u8s(char *src)
 {
    int srcLen = 0;
    if ((srcLen = strlen(src)) == 0)
        return NULL;

    int wcsLen = srcLen + 1;
    wchar_t wcs[wcsLen];
    wmemset(wcs, 0, srcLen);

    if (MultiByteToWideChar(CP_ACP, 0, src, -1, wcs, wcsLen) == 0)
        return NULL;

    int u8sLen = 0;
    if ((u8sLen = WideCharToMultiByte(CP_UTF8, 0, wcs, -1, NULL, 0, NULL, NULL)) == 0)
        return NULL;

    char *u8s;
    if ((u8s = (char*)malloc(u8sLen * sizeof(char))) == NULL)
        return NULL;
    
    if (WideCharToMultiByte(CP_UTF8, 0, wcs, -1, u8s, u8sLen, NULL, NULL) == 0)
    {

        free(u8s);
        return NULL;
    }

    return u8s;
 }
#endif
