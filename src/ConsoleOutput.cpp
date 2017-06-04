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

#include "ConsoleOutput.h"

#include <fstream>
#include <iostream>

#include "Model.h"
#include "Region.h"
#include "Sector.h"

ConsoleOutput::ConsoleOutput(XMLInput* settings, Model* model, Scenario* scenario, pugi::xml_node output_node)
    : Output(settings, model, scenario, output_node) {
    stack = 0;
    out = 0;
}

ConsoleOutput::~ConsoleOutput() {
    if (out != &std::cout) {
        delete out;
    }
}

int ConsoleOutput::initialize() {
    if (output_node.attribute("file").empty()) {
        out = &std::cout;
    } else {
        std::string filename = output_node.attribute("file").value();
        out = new std::ofstream();
        ((std::ofstream*)out)->open(filename.c_str(), std::ofstream::out);
    }
    return 0;
}

void ConsoleOutput::internal_write_header(tm* timestamp, int max_threads, const char* version) {
    *out << "Start time " << asctime(timestamp) << "Version " << version << "\n"
         << "Max " << max_threads << " threads" << std::endl;
}

void ConsoleOutput::internal_write_footer(tm* duration) { *out << "\n\nDuration " << mktime(duration) << "s"; }

void ConsoleOutput::internal_write_xml_input(XMLInput* input) {
    *out << std::endl;
    input->print(std::wcout, "  ", pugi::format_indent | pugi::format_no_declaration, 0);
    *out << std::endl;
}

void ConsoleOutput::internal_start() { *out << "Starting"; }

void ConsoleOutput::internal_iterate_begin() { *out << std::endl << std::endl << "Iteration time " << model->time; }

void ConsoleOutput::internal_end() { *out << std::endl << std::endl << "Ended" << std::endl; }

void ConsoleOutput::internal_write_double(const char* name, double v) { *out << "\t" << name << "=" << v; }

void ConsoleOutput::internal_start_target(const char* name, Sector* sector, Region* region) {
    stack++;
    *out << std::endl << std::string(2 * stack, ' ') << name << " " << sector->name << "," << region->name << ":";
}

void ConsoleOutput::internal_start_target(const char* name, Sector* sector) {
    stack++;
    *out << std::endl << std::string(2 * stack, ' ') << name << " " << sector->name << ":";
}

void ConsoleOutput::internal_start_target(const char* name, Region* region) {
    stack++;
    *out << std::endl << std::string(2 * stack, ' ') << name << " " << region->name << ":";
}

void ConsoleOutput::internal_start_target(const char* name) {
    stack++;
    *out << std::endl << std::string(2 * stack, ' ') << name << ":";
}

void ConsoleOutput::internal_end_target() { stack--; }
