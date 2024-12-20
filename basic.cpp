#include "headers/bsp.hpp"
#include "headers/bspdefs.hpp"
#include <iostream>
#include <stdlib.h>
#include <vector>

// For future error handling.
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
#define ERRORINVD(x, func)         \
    if ((void *)(x) == (void *)-1) \
    {                              \
        perror(#func);             \
        abort();                   \
    }

#define USAGE \
"Usage: %s BSP_INPUT\n"

#define PROP_VERSION 10

std::ostream& operator<<(std::ostream& os, const dbrushside_t& brushside) {
    os << "planenum: " << brushside.planenum << ", texture info: " << brushside.texinfo << ", displacement info: " << brushside.dispinfo << ", bevel: " << (brushside.bevel == 1 ? "true" : "false");
    return os;
}

std::ostream& operator<<(std::ostream& os, const dbrush_t& brush) {
    os << "flags: " << brush.contents << ", brushsides index: " << brush.firstside << ", count: " << brush.numsides;
    return os;
}

std::ostream& operator<<(std::ostream& os, const dgamelump_t& gamelump) {
    char id[5] = {((char *)&gamelump.id)[3], ((char *)&gamelump.id)[2], ((char *)&gamelump.id)[1], ((char *)&gamelump.id)[0], '\0'};
    os << "id: " << id << ", version: " << gamelump.version << ", flags: " << gamelump.flags << ", size: " << gamelump.filelen;
    return os;
}

int main (int argc, char **argv)
{
    if (argc != 2)
    {
        printf(USAGE, argv[0]);
        return 1;
    }
    Bsp input(argv[1]);

    input.SelectLump<dbrush_t>(LUMP_BRUSHES);
    std::vector<dbrush_t> brushes = input.GetAllLumpElements<dbrush_t>();
    int brushnum = input.GetElementCount();
    input.SelectLump<dbrushside_t>(LUMP_BRUSHSIDES);
    std::vector<dbrushside_t> brushsides = input.GetAllLumpElements<dbrushside_t>();
    int brushsidenum = input.GetElementCount();

    int brushid = 0;
    for (const auto& brush : brushes)
    {
        std::cout << "Brush " << brushid << ": " << brush << "\n";
        for (int i = 0; i < brush.numsides; i++)
        {
            std::cout << "Side " << i << ": " << brushsides[brush.firstside + i] << "\n";
        }
        std::cout << "\n";
        brushid++;
    }

    std::vector<dgamelump_t> gamelumps = input.GetAllGameLumps();
    int gameid = 0;
    for (const auto &gamelump : gamelumps)
    {
        std::cout << "Gamelump " << gameid << ": " << gamelump << "\n";
        gameid++;
    }
    std::cout << "\n";

    std::cout << "Number of brushes: " << brushnum << "\n";
    std::cout << "Number of brushsides: " << brushsidenum << "\n";
    std::cout << "Number of visclusers: " << input.GetVisClusterCount() << "\n";
    std::cout << "Number of gamelumps: " << input.GetGameLumpCount() << "\n";

    std::cout << std::endl << "Finished" << std::endl;

    return 0;
}
