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

#include <csignal>
#include <exception>
#include <iostream>
#include <vector>

#include "CSVModelInitializer.h"
#include "ConsoleOutput.h"
#include "GnuplotOutput.h"
#include "HistogramOutput.h"
#include "JSONOutput.h"
#include "Model.h"
#include "ModelInitializer.h"
#include "NetCDFOutput.h"
#include "Output.h"
#include "Scenario.h"
#include "XMLInput.h"

Model* model;
Scenario* scenario;
pugi::xml_document settings_document;
XMLInput settings_nodes;

void insert_xml_to_settings(pugi::xml_node node) {
    if (!node.empty()) {
        pugi::xml_node parent = settings_nodes.child(node.name());
        if (parent.empty()) {
            parent = settings_nodes.prepend_copy(node);
        } else {
            for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling()) {
                parent.append_copy(child);
            }
        }
    }
}

int load_settings(const char* settings_filename) {
    pugi::xml_parse_result result;
    result = settings_document.load_file(settings_filename);
    if (!result) {
        std::cerr << "ERROR in '" << settings_filename << "': " << result.description() << std::endl;
        return -2;
    }

    settings_nodes = settings_document.child("acclimate");

    const unsigned char required_children_count = 4;
    const char* required_children[required_children_count] = {"network", "parameters", "scenario", "output"};

    for (pugi::xml_node node = settings_nodes.child("settings"); node; node = node.next_sibling("settings")) {
        const char* filename = node.attribute("file").value();
        pugi::xml_document doc;
        result = doc.load_file(filename);
        if (!result) {
            std::cerr << "ERROR in '" << filename << "': " << result.description() << std::endl;
            return -2;
        }
        for (int i = 0; i < required_children_count; i++) {
            for (pugi::xml_node node = doc.child(required_children[i]); node; node = node.next_sibling(required_children[i])) {
                insert_xml_to_settings(node);
            }
        }
    }

    for (int i = 0; i < required_children_count; i++) {
        if (settings_nodes.child(required_children[i]).empty()) {
            std::cerr << "ERROR: No " << required_children[i] << " configuration given" << std::endl;
            return -1;
        }
    }
    return 0;
}

int initialize() {
    // Initialize model
    model = new Model();

    ModelInitializer* model_initializer;
    std::string type = settings_nodes.child("network").attribute("type").value();
    if (type == "csv") {
        model_initializer = new CSVModelInitializer(model, &settings_nodes);
    } else {
        std::cerr << "ERROR: Unknown network type '" << type << "'" << std::endl;
        return -2;
    }
    int res = model_initializer->initialize();
    if (res != 0) {
        return res;
    }
    delete model_initializer;

    // Initialize scenario
    scenario = new Scenario(&settings_nodes, model);

    // Initialize output
    for (pugi::xml_node node = settings_nodes.child("output"); node; node = node.next_sibling("output")) {
        Output* output;
        type = node.attribute("format").value();
        if (type == "console") {
            output = new ConsoleOutput(&settings_nodes, model, scenario, node);
        } else if (type == "json") {
            output = new JSONOutput(&settings_nodes, model, scenario, node);
        } else if (type == "netcdf") {
            output = new NetCDFOutput(&settings_nodes, model, scenario, node);
        } else if (type == "histogram") {
            output = new HistogramOutput(&settings_nodes, model, scenario, node);
        } else if (type == "gnuplot") {
            output = new GnuplotOutput(&settings_nodes, model, scenario, node);
        } else {
            std::cerr << "ERROR: Unknown output format '" << type << "'" << std::endl;
            return -2;
        }

        res = output->initialize();
        if (res != 0) {
            return res;
        }

        model->outputs.push_back(output);
    }

    return 0;
}

void run() {
    model->time = scenario->start_time;

    for (int i = 0; i < model->outputs.size(); i++) {
        model->outputs[i]->start();
    }
    while (scenario->iterate()) {
        model->iterate();
        for (int i = 0; i < model->outputs.size(); i++) {
            model->outputs[i]->iterate();
        }
        model->time += model->delta_t;
    }
    for (int i = 0; i < model->outputs.size(); i++) {
        model->outputs[i]->end();
    }
}

void cleanup() {
    delete scenario;
    delete model;
}

void handle_fpe_error(int a) {
    std::cerr << "FPE Error " << a << std::endl;
    exit(-1);
}

int main(int argc, char* argv[]) {
    signal(SIGFPE, handle_fpe_error);
    try {
        int res;

        if (argc > 1) {
            res = load_settings(argv[1]);
        } else {
            res = load_settings("settings.xml");
        }
        if (res != 0) {
            cleanup();
            return res;
        }

        res = initialize();
        if (res != 0) {
            cleanup();
            return res;
        }

        run();

        cleanup();

        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << std::endl;
        return -5;
    }
}
