/*
  Copyright (C) 2013-2014 Sven Willner <sven.willner@pik-potsdam.de>

  This file is part of Acclimate (basic and demand version).

  Acclimate is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  Acclimate is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with Acclimate.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "JSONOutput.h"

#include <iostream>
#include "Model.h"
#include "Region.h"
#include "Scenario.h"
#include "Sector.h"

JSONOutput::JSONOutput(XMLInput* settings, Model* model, Scenario* scenario, pugi::xml_node output_node) : Output(settings, model, scenario, output_node) {}

int JSONOutput::initialize() {
    if (output_node.attribute("file").empty()) {
        std::cerr << "ERROR: Output file name not given" << std::endl;
        return -1;
    }
    std::string filename = output_node.attribute("file").value();
    out.open(filename.c_str(), std::ofstream::out);
    return 0;
}

void JSONOutput::internal_write_header(tm* timestamp, int max_threads, const char* version) {
    out << "var output = {\n";
    out << "\"info_header\": {\n";
    out << "    \"start_time\": \"" << asctime(timestamp);
    out.seekp(-1, std::ios_base::end);
    out << "\",\n";
    out << "    \"version\": \"" << version << "\",\n";
    out << "    \"max_threads\": " << max_threads << ",\n";
}

void JSONOutput::internal_write_footer(tm* duration) {
    out.seekp(-2, std::ios_base::end);
    out << "\n},\n";
    out << "\"info_footer\": {\n";
    out << "    \"duration\": \"" << mktime(duration) << "s\"\n";
    out << "}\n";
}

void JSONOutput::internal_write_xml_input(XMLInput* input) {
    std::stringstream ss;
    input->print(ss, "  ", pugi::format_indent | pugi::format_no_declaration, pugi::encoding_utf8, 0);
    ss.flush();
    ss.seekg(0);
    std::string line;
    out << "    \"settings\": '";
    while (std::getline(ss, line)) {
        out << line << "\\\n";
    }
    out << "'\n},\n";
    out << "\"data\": {";
}

void JSONOutput::internal_start() {}

void JSONOutput::internal_iterate_begin() { out << "\n    \"" << model->time << "\": {\n"; }

void JSONOutput::internal_iterate_end() {
    out.seekp(-2, std::ios_base::end);
    out << "\n    },\n";
}

void JSONOutput::internal_end() {
    out << "\n};\n";
    out.close();
}

void JSONOutput::internal_write_double(const char* name, double v) { out << "            \"" << name << "\": " << v << ",\n"; }

void JSONOutput::internal_end_target() {
    out.seekp(-2, std::ios_base::end);
    out << "\n        },\n";
}

void JSONOutput::internal_start_target(const char* name, Sector* sector, Region* region) {
    out << "        \"" << name << "\": {\n";
    out << "            \"sector\": \"" << sector->name << "\",\n";
    out << "            \"region\": \"" << region->name << "\",\n";
}

void JSONOutput::internal_start_target(const char* name, Sector* sector) {
    out << "        \"" << name << "\": {\n";
    out << "            \"sector\": \"" << sector->name << "\",\n";
}

void JSONOutput::internal_start_target(const char* name, Region* region) {
    out << "        \"" << name << "\": {\n";
    out << "            \"region\": \"" << region->name << "\",\n";
}

void JSONOutput::internal_start_target(const char* name) { out << "        \"" << name << "\": {\n"; }
