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

#include "Region.h"

#include "EconomicAgent.h"
#include "Model.h"
#include "PurchasingManager.h"
#include "SalesManager.h"
#include "Storage.h"

Region::Region(Model* model, std::string name) : GeographicEntity(REGION), model(model), name(name) {
    export_flow__Z[0] = 0;
    export_flow__Z[1] = 0;
    import_flow__Z[0] = 0;
    import_flow__Z[1] = 0;
    consumption_flow__Y[0] = 0;
    consumption_flow__Y[1] = 0;
}

Region::~Region() {
    for (int i = 0; i < economic_agents.size(); i++) {
        delete economic_agents[i];
    }
}

void Region::add_export__Z(Flow export_flow__Z) {
#pragma omp atomic
    this->export_flow__Z[model->current_register] += export_flow__Z;
}

void Region::add_import__Z(Flow import_flow__Z) {
#pragma omp atomic
    this->import_flow__Z[model->current_register] += import_flow__Z;
}

void Region::add_consumption_flow__Y(Flow consumption_flow__Y) {
#pragma omp atomic
    this->consumption_flow__Y[model->current_register] += consumption_flow__Y;
}

Quantity Region::current_gdp() {
    return consumption_flow__Y[model->current_register] + export_flow__Z[model->current_register] - import_flow__Z[model->current_register];
}

void Region::iterate_consumption_and_production() {
    export_flow__Z[model->other_register] = 0;
    import_flow__Z[model->other_register] = 0;
    consumption_flow__Y[model->other_register] = 0;
    int i;
#pragma omp parallel default(shared) private(i)
    {
#pragma omp for schedule(guided) nowait
        for (i = 0; i < economic_agents.size(); i++) {
            economic_agents[i]->iterate_consumption_and_production();
        }
    }
}

Flow Region::current_consumption__C() { return consumption_flow__Y[model->current_register]; }

Flow Region::current_import_flow__Z() { return import_flow__Z[model->current_register]; }

Flow Region::current_export_flow__Z() { return import_flow__Z[model->current_register]; }

void Region::iterate_demand() {
    int i;
#pragma omp parallel default(shared) private(i)
    {
#pragma omp for schedule(guided) nowait
        for (i = 0; i < economic_agents.size(); i++) {
            economic_agents[i]->iterate_demand();
        }
    }
}

void Region::find_path_to(Region* region, Path* path) {
    // preliminary, assuming all regions are connected by infrastructures
    path->distance = 0;
    for (std::vector<GeographicEntity*>::iterator it = connections.begin(); it != connections.end(); it++) {
        Infrastructure* inf = (Infrastructure*)(*it);
        if ((inf->connections.size() > 0 && (void*)inf->connections[0] == (void*)region)
            || (inf->connections.size() > 1 && (void*)inf->connections[1] == (void*)region)) {
            path->infrastructures.push_back(inf);
            path->distance = inf->distance;
            break;
        }
    }
}

void Region::remove_economic_agent(EconomicAgent* economic_agent) {
    for (std::vector<EconomicAgent*>::iterator it = economic_agents.begin(); it != economic_agents.end(); it++) {
        if (*it == economic_agent) {
            economic_agents.erase(it);
            break;
        }
    }
}
