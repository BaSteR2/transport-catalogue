#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "graph.h"
#include "router.h"
#include  "serialization.h"
#include <transport_catalogue.pb.h>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <filesystem>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {      
        reader::JsonReader reader(std::cin);
        reader.FillDataFromJson();
        reader.SerializeData();

    }
    else if (mode == "process_requests"sv) {        
        reader::JsonReader reader(std::cin);
        reader.DeserializeData();
        reader.PrintRequest(std::cout);
    }
    else {
        PrintUsage();
        return 1;
    }
}
