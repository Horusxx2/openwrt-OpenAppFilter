/*
Copyright (C) 2020 Derry <destan19@126.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "appfilter_config.h"
#include <uci.h>

app_name_info_t app_name_table[MAX_SUPPORT_APP_NUM];
int g_app_count = 0;
int g_cur_class_num = 0;
char CLASS_NAME_TABLE[MAX_APP_TYPE][MAX_CLASS_NAME_LEN];

const char *config_path = "./config";
static struct uci_context *uci_ctx = NULL;
static struct uci_package *uci_appfilter;



//00:00 9:1
int check_time_valid(char *t)
{
    if (!t)
        return 0;
    if (strlen(t) < 3 || strlen(t) > 5 || (!strstr(t, ":")))
        return 0;
    else
        return 1;
}

void dump_af_time(af_ctl_time_t *t)
{
    int i;
    printf("---------dump af time-------------\n");
    printf("%d:%d ---->%d:%d\n", t->start.hour, t->start.min,
           t->end.hour, t->end.min);
    for (i = 0; i < 7; i++)
    {
        printf("%d ", t->days[i]);
    }
    printf("\n");
}

af_ctl_time_t *load_appfilter_ctl_time_config(void)
{
    char start_time_str[64] = {0};
    char end_time_str[64] = {0};
    char days_str[64] = {0};
    int ret = 0;
    af_ctl_time_t *t = NULL;
    struct uci_context *ctx = uci_alloc_context();
    if (!ctx)
        return NULL;

    ret |= uci_get_value(ctx, "appfilter.time.start_time", start_time_str, sizeof(start_time_str));
    ret |= uci_get_value(ctx, "appfilter.time.end_time", end_time_str, sizeof(end_time_str));
    ret |= uci_get_value(ctx, "appfilter.time.days", days_str, sizeof(days_str));
    if (ret != 0){
        printf("time config error\n");
        return NULL;
    }

    if (!check_time_valid(start_time_str) || !check_time_valid(end_time_str)){
        printf("format error\n");
        return NULL;
    }
    t = malloc(sizeof(af_ctl_time_t));

    sscanf(start_time_str, "%d:%d", &t->start.hour, &t->start.min);
    sscanf(end_time_str, "%d:%d", &t->end.hour, &t->end.min);

    char *p = strtok(days_str, " ");
    if (!p)
        goto EXIT;
    do
    {
        int day = atoi(p);
        if (day >= 0 && day <= 6)
            t->days[day] = 1;
        else
            ret = 0;
    } while (p = strtok(NULL, " "));
EXIT:
	uci_free_context(ctx);
    return t;
}



int config_get_appfilter_enable(void)
{
    int enable = 0;
    struct uci_context *ctx = uci_alloc_context();
    if (!ctx)
        return NULL;
	enable = uci_get_int_value(ctx, "appfilter.global.enable");
    if (enable < 0)
        enable = 0;
    
	uci_free_context(ctx);
    return enable;
}

int appfilter_config_alloc(void)
{
    char *err;
    uci_appfilter = config_init_package("appfilter");
    if (!uci_appfilter)
    {
        uci_get_errorstr(uci_ctx, &err, NULL);
        printf("Failed to load appfilter config (%s)\n", err);
        free(err);
        return -1;
    }

    return 0;
}

int appfilter_config_free(void)
{
    if (uci_appfilter)
    {
        uci_unload(uci_ctx, uci_appfilter);
        uci_appfilter = NULL;
    }
    if (uci_ctx)
    {
        uci_free_context(uci_ctx);
        uci_ctx = NULL;
    }
}
