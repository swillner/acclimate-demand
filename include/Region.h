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

#ifndef _REGION_H
#define _REGION_H

#include <string>

#include "GeographicEntity.h"
#include "Infrastructure.h"
#include "types.h"

class Model;
class EconomicAgent;

typedef struct {
    Distance distance;
    std::vector<Infrastructure*> infrastructures;
} Path;

class Region : public GeographicEntity {
  protected:
    Flow export_flow__Z[2];
    Flow import_flow__Z[2];
    Flow consumption_flow__Y[2];

  public:
    std::vector<EconomicAgent*> economic_agents;
    std::string name;
    Model* const model;

  public:
    Region(Model* model, std::string name);
    ~Region();
    void add_export__Z(Flow export_flow__Z);
    void add_import__Z(Flow import_flow__Z);
    void add_consumption_flow__Y(Flow consumption_flow__Y);
    Quantity current_gdp();
    void iterate_consumption_and_production();
    void iterate_demand();
    void find_path_to(Region* region, Path* path);
    Flow current_consumption__C();
    Flow current_import_flow__Z();
    Flow current_export_flow__Z();
    void remove_economic_agent(EconomicAgent* economic_agent);
};

#endif
