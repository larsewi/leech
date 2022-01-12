#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <leech.h>

enum {
    LOG_LEVEL_NONE,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_VERBOSE,
    LOG_LEVEL_DEBUG,
};

static int LOG_LEVEL = LOG_LEVEL_INFO;
static char *BOOTSTRAP_ADDRESS = NULL;

static void CheckOpts(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    CheckOpts(argc, argv);
    PrintHello(__BASE_FILE__);
    return 0;
}

static void CheckOpts(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "b:hl:")) != -1)
    {
        switch (opt)
        {
        case 'b':
            BOOTSTRAP_ADDRESS = optarg;
            break;

        case 'h':
            printf("%s: [OPTION]...", argv[0]);
            exit(EXIT_SUCCESS);
            break;

        case 'l':
            const char *log_levels[] = {
                "none", "error", "warning", "info", "verbose", "debug"
            };
            for (int lvl = LOG_LEVEL_DEBUG; lvl <= LOG_LEVEL_NONE; lvl++)
            {
                if (strcasecmp(optarg, log_levels[lvl]) == 0)
                {
                    LOG_LEVEL = lvl;
                }
            }
            break;

        default:
            fprintf(stderr, "%s: [OPTION]...", argv[0]);
            exit(EXIT_FAILURE);
            break;
        }
    }
}
