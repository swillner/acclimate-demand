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

#include "GnuplotOutput.h"

#include <fstream>

#include "Model.h"
#include "Region.h"
#include "Sector.h"

GnuplotOutput::GnuplotOutput(XMLInput* settings, Model* model, Scenario* scenario, pugi::xml_node output_node)
    : Output(settings, model, scenario, output_node) {}

int GnuplotOutput::initialize() {
    if (output_node.attribute("file").empty()) {
        std::cerr << "ERROR: Output file name not given" << std::endl;
        return -1;
    }
    std::string filename = output_node.attribute("file").value();
    file.open(filename.c_str(), std::ofstream::out);
    return 0;
}

void GnuplotOutput::internal_write_header(tm* timestamp, int max_threads, const char* version) {
    file << "# Start time: " << asctime(timestamp) << "# Version: " << version << "\n"
         << "# Max number of threads: " << max_threads << "\n";
}

void GnuplotOutput::internal_write_footer(tm* duration) { file << "# Duration: " << mktime(duration) << "s\n"; }

void GnuplotOutput::internal_write_xml_input(XMLInput* input) {
    std::stringstream ss;
    input->print(ss, "  ", pugi::format_indent | pugi::format_no_declaration, pugi::encoding_utf8, 0);
    ss.flush();
    ss.seekg(0);
    std::string line;
    file << "# Settings:\n";
    while (std::getline(ss, line)) {
        file << "# " << line << "\n";
    }
    file << "#\n";
}

void GnuplotOutput::internal_end() { file.close(); }

void GnuplotOutput::internal_start() {
    file << "# Sectors:\n#set ytics(";
    for (size_t i = 0; i < model->sectors__C.size(); i++) {
        file << "\"" << model->sectors__C[i]->name << "\" " << i;
        if (i < model->sectors__C.size() - 1) {
            file << ", ";
        }
        sector_index.insert(std::make_pair(model->sectors__C[i], i));
    }
    file << ")\n#Regions:\n#set ytics(";
    for (size_t i = 0; i < model->regions__R.size(); i++) {
        file << "\"" << model->regions__R[i]->name << "\" " << i;
        if (i < model->regions__R.size() - 1) {
            file << ", ";
        }
        region_index.insert(std::make_pair(model->regions__R[i], i));
    }
    file << ")\n";
}

void GnuplotOutput::internal_write_double(const char* name, double v) {
    file << model->time << " ";
    for (auto it = stack.begin(); it != stack.end(); it++) {
        if (it->region >= 0) {
            if (it->sector >= 0) {
                file << it->sector << " " << it->region << " ";
            } else {
                file << it->region << " ";
            }
        } else {
            if (it->sector >= 0) {
                file << it->sector << " ";
            }
        }
    }
    file << v << "\n";
}

void GnuplotOutput::internal_start_target(const char* name, Sector* sector, Region* region) {
    Target t;
    t.sector = sector_index[sector];
    t.region = region_index[region];
    stack.push_back(t);
}

void GnuplotOutput::internal_start_target(const char* name, Sector* sector) {
    Target t;
    t.sector = sector_index[sector];
    t.region = -1;
    stack.push_back(t);
}

void GnuplotOutput::internal_start_target(const char* name, Region* region) {
    Target t;
    t.sector = -1;
    t.region = region_index[region];
    stack.push_back(t);
}

void GnuplotOutput::internal_start_target(const char* name) {
    Target t;
    t.sector = -1;
    t.region = -1;
    stack.push_back(t);
}

void GnuplotOutput::internal_end_target() { stack.pop_back(); }
