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

#ifndef _CSVMODELINITIALIZER_H
#define _CSVMODELINITIALIZER_H

#include "ModelInitializer.h"
#include "XMLInput.h"

class Model;

class CSVModelInitializer : public ModelInitializer {
  protected:
    Distance transport_time;
    int read_index_file(std::string index_filename, std::vector<EconomicAgent*>* economic_agents);
    int read_flows(std::string filename, std::vector<EconomicAgent*>* economic_agents_row, std::vector<EconomicAgent*>* economic_agents_col);
    int read_flows(std::string index_filename, std::string filename);
    int read_flows(std::string row_index_filename, std::string col_index_filename, std::string filename);
    int read_transport_times(std::string index_filename, std::string filename);

  public:
    CSVModelInitializer(Model* model, XMLInput* settings);
    int initialize();
};

#endif
