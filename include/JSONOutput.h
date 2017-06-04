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

#ifndef _JSONOUTPUT_H
#define _JSONOUTPUT_H

#include <fstream>
#include <sstream>

#include "Output.h"
#include "XMLInput.h"

class Model;

class JSONOutput : public Output {
  protected:
    std::ofstream out;
    void internal_write_header(tm* timestamp, int max_threads, const char* version);
    void internal_write_footer(tm* duration);
    void internal_write_xml_input(XMLInput* input);
    void internal_start();
    void internal_iterate_begin();
    void internal_iterate_end();
    void internal_end();
    void internal_write_double(const char* name, double v);
    void internal_start_target(const char* name, Sector* sector, Region* region);
    void internal_start_target(const char* name, Sector* sector);
    void internal_start_target(const char* name, Region* region);
    void internal_start_target(const char* name);
    void internal_end_target();

  public:
    JSONOutput(XMLInput* settings, Model* model, Scenario* scenario, pugi::xml_node output_node);
    int initialize();
};

#endif
