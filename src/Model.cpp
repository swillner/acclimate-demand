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

#include "Model.h"

#include "EconomicAgent.h"
#include "FinalDemand.h"
#include "Infrastructure.h"
#include "ProductionSite.h"
#include "Region.h"
#include "Sector.h"

Model::Model() {
    time = 0;
    current_register = 1;
    other_register = 0;
    delta_t = 1;
}

Model::~Model() {
    for (int i = 0; i < sectors__C.size(); i++) {
        delete sectors__C[i];
    }

    for (int i = 0; i < regions__R.size(); i++) {
        delete regions__R[i];
    }

    for (int i = 0; i < infrastructure__G.size(); i++) {
        delete infrastructure__G[i];
    }

    for (int i = 0; i < outputs.size(); i++) {
        delete outputs[i];
    }
}

void Model::iterate() {
    int tmp = current_register;
    current_register = other_register;
    other_register = tmp;

    int i;

#pragma omp parallel default(shared) private(i)
    {
#pragma omp for schedule(guided) nowait
        for (i = 0; i < sectors__C.size(); i++) {
            sectors__C[i]->iterate_consumption_and_production();
        }
    }

#pragma omp parallel default(shared) private(i)
    {
#pragma omp for schedule(guided) nowait
        for (i = 0; i < regions__R.size(); i++) {
            regions__R[i]->iterate_consumption_and_production();
        }
    }

#pragma omp parallel default(shared) private(i)
    {
#pragma omp for schedule(guided) nowait
        for (i = 0; i < regions__R.size(); i++) {
            regions__R[i]->iterate_demand();
        }
    }
}

Region* Model::find_region(std::string name) {
    for (std::vector<Region*>::const_iterator it = regions__R.begin(); it != regions__R.end(); it++) {
        if ((*it)->name == name) {
            return *it;
        }
    }
    return 0;
}

Sector* Model::find_sector(std::string name) {
    for (std::vector<Sector*>::const_iterator it = sectors__C.begin(); it != sectors__C.end(); it++) {
        if ((*it)->name == name) {
            return *it;
        }
    }
    return 0;
}

ProductionSite* Model::find_production_site(std::string sector_name, std::string region_name) {
    Sector* sector = find_sector(sector_name);
    if (sector) {
        return find_production_site(sector, region_name);
    } else {
        return 0;
    }
}

ProductionSite* Model::find_production_site(Sector* sector, std::string region_name) {
    for (std::vector<ProductionSite*>::const_iterator it = sector->production_sites__N.begin(); it != sector->production_sites__N.end(); it++) {
        if ((*it)->region->name == region_name) {
            return *it;
        }
    }
    return 0;
}

FinalDemand* Model::find_final_demand(Region* region) {
    for (std::vector<EconomicAgent*>::const_iterator it = region->economic_agents.begin(); it != region->economic_agents.end(); it++) {
        if ((*it)->type == FINAL_DEMAND) {
            return (FinalDemand*)(*it);
        }
    }
    return 0;
}

FinalDemand* Model::find_final_demand(std::string region_name) {
    Region* region = find_region(region_name);
    if (region) {
        return find_final_demand(region);
    }
    return 0;
}
