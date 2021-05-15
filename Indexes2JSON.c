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

// mkdnotes --commit 

#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include "./vendor/cJSON/cJSON.c"

char* Idx2JSON_printStr(char *startDirPath);
void Idx2JSON_recursiveDir(char *dirPath, cJSON *f);

char* Idx2JSON_printStr(char *startDirPath)
{
	cJSON *root = NULL;
	if ((root = cJSON_CreateArray()) == NULL)
	{
		fprintf(stderr, "error: Failed to create cJSON structure!\n");
		exit(1);
	}

	Idx2JSON_recursiveDir(startDirPath, root);
	
	char *res = NULL;
	if ((res = cJSON_Print(root)) == NULL)
	{
		fprintf(stderr, "error: Failed to print cJSON structure!\n");
		exit(1);
	}
	
	cJSON_Delete(root);

	return res;
}

void Idx2JSON_recursiveDir(char *dirPath, cJSON *f)
{
	DIR *dirP = NULL;
	if ((dirP = opendir(dirPath)) == NULL)
	{
		fprintf(stderr, "error: Failed to open directory: \"%s\"\n", dirPath);
		exit(1);
	}

	struct dirent *dp;
	dp = readdir(dirP);
	
	while (dp)
	{
		if (errno != 0)
		{
			fprintf(stderr, "error: Failed to read directory: \"%s\"\n", dirPath);
			exit(1);
		}

		if (strcmp(dp->d_name, ".") && strcmp(dp->d_name, ".."))
		{
			// check the file type
			char testFilePath[256] = {'\0'};
			strcpy(testFilePath, dirPath);
			strcat(testFilePath, "/");
			strcat(testFilePath, dp->d_name);

			struct stat buf;
			stat(testFilePath, &buf);
			if (S_ISDIR(buf.st_mode))
			{
				// is folder
				cJSON *obj = NULL,
					  *arr = NULL;

				if ((obj = cJSON_CreateObject()) == NULL || (arr = cJSON_CreateArray()) == NULL || (cJSON_AddStringToObject(obj, "text", dp->d_name)) == NULL)
				{
					fprintf(stderr, "error: Failed to create cJSON structure!\n");
					exit(1);
				}

				cJSON_AddItemToObject(obj, "nodes", arr);
				cJSON_AddItemToArray(f, obj);

				// sub folder
				Idx2JSON_recursiveDir(testFilePath, arr);
			}
			else if (strlen(dp->d_name) > 5 && strcmp(dp->d_name + (strlen(dp->d_name) - 5), ".html") == 0)
			{
				cJSON *obj = NULL;
				if ((obj = cJSON_CreateObject()) == NULL || (cJSON_AddStringToObject(obj, "text", dp->d_name)) == NULL)
				{
					fprintf(stderr, "error: Failed to create cJSON structure!\n");
					exit(1);
				}

				cJSON_AddItemToArray(f, obj);
			}
		}

		dp = readdir(dirP);
	}

	closedir(dirP);
}
