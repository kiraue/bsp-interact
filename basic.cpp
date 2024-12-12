#include "headers/bsp.hpp"
#include "headers/bspdefs.hpp"
#include <iostream>
#include <stdlib.h>
#include <memory>

#define ERROR(x, func) \
    if ((x) < 0)       \
    {                  \
        perror(#func); \
        abort();       \
    }
#define ERRORNULL(x, func)             \
    if ((x) == NULL || (x) == nullptr) \
    {                                  \
        perror(#func);                 \
        abort();                       \
    }
#define ERRORINVD(x, func)      \
    if ((void *)(x) == (void *)-1) \
    {                           \
        perror(#func);          \
        abort();                \
    }

#define USAGE \
"Usage: %s BSP_INPUT\n"

#define PROP_VERSION 10

int main (int argc, char **argv)
{
    if (argc != 2)
    {
        printf(USAGE, argv[0]);
        return 1;
    }
    Bsp input(argv[1]);
    std::cout << "Backing up file...\n";
    input.Backup(".old", (1 << 25));
    std::cout << "File backed up\n";

    input.SelectLump<dbrush_t>(LUMP_BRUSHES);

    std::unique_ptr<dbrush_t[]> brushes(input.GetAllLumpElements<dbrush_t>());

    for (int i = 0; i < input.GetElementCount(); i++)
    {
        dbrush_t& brush = brushes[i];
        std::cout << "Index: " << i << "\n";
        std::cout << "Number of brushsides: " << brush.numsides << "\n";
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
