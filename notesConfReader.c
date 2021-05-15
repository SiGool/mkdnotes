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

// mkdnotes --init -o xxx

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./vendor/inih/ini.c"

typedef struct {
    const char *title;
    const char *link_icon;
    const char *theme;
} configuration;

configuration getConfOptions(char confFilePath[]);
static int handler(void *user, const char *section, const char *name, const char *value);

configuration getConfOptions(char confFilePath[])
{
    configuration conf;

    if (ini_parse(confFilePath, handler, &conf) < 0)
    {
        fprintf(stderr, "error: failed to load configuration file\n");
        exit(1); 
    }

    if (conf.title == NULL || conf.link_icon == NULL || conf.theme == NULL)
    {
        fprintf(stderr, "error: Configuration file has error\n");
        exit(1); 
    }
    
    return conf;
}

static int handler(void *user, const char *section, const char *name, const char *value)
{
    configuration *confP = (configuration*)user;

    static int initalized = 0;
    if (!initalized++)
    {
        confP->title = NULL;
        confP->link_icon = NULL;
        confP->theme = NULL; 
    }

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

    if (MATCH("notes", "title")) 
        confP->title = strdup(value);
    else if (MATCH("notes", "link_icon"))
    { 
        confP->link_icon = strdup(value);
        if (strcmp(confP->link_icon, "Yes") && strcmp(confP->link_icon, "No"))
        {
            fprintf(stderr, "error: Configuration file has error\n");
            exit(1); 
        }
    }
    else if (MATCH("notes", "theme"))
        confP->theme = strdup(value);
    else
        return 0;
    
    return 1;
}