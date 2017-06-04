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

#ifndef _HISTOGRAMOUTPUT_H
#define _HISTOGRAMOUTPUT_H

#include <fstream>
#include <iostream>
#include <sstream>

#include "Output.h"
#include "XMLInput.h"

class Model;

class HistogramOutput : public Output {
  private:
    std::ofstream file;
    bool exclude_max;
    unsigned short windows;
    double min, max;
    unsigned int* count;

  protected:
    void internal_write_header(tm* timestamp, int max_threads, const char* version);
    void internal_write_footer(tm* duration);
    void internal_write_xml_input(XMLInput* input);
    void internal_iterate_begin();
    void internal_iterate_end();
    void internal_end();
    void internal_write_double(const char* name, double v);

  public:
    HistogramOutput(XMLInput* settings, Model* model, Scenario* scenario, pugi::xml_node output_node);
    int initialize();
};

#endif
