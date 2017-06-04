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

#include "EconomicAgent.h"

#include "OutputStorage.h"
#include "ProductionSite.h"
#include "PurchasingManager.h"
#include "Region.h"
#include "Sector.h"
#include "Storage.h"

EconomicAgent::EconomicAgent(Region* region, EconomicAgentType type) : region(region), type(type) {}

EconomicAgent::~EconomicAgent() {
    for (int i = 0; i < input_storages.size(); ++i) {
        if (type != PRODUCTION_SITE || (void*)input_storages[i] != (void*)((ProductionSite*)this)->output_storage) {
            delete input_storages[i];
        }
    }
}

Storage* EconomicAgent::find_input_storage(std::string sector_name) {
    for (std::vector<Storage*>::const_iterator it = input_storages.begin(); it != input_storages.end(); it++) {
        if ((*it)->sector->name == sector_name) {
            return *it;
        }
    }
    return 0;
}

void EconomicAgent::iterate_demand() {
    for (int j = 0; j < input_storages.size(); j++) {
        input_storages[j]->purchasing_manager->iterate_demand();
    }
}

void EconomicAgent::remove_storage(Storage* storage) {
    for (std::vector<Storage*>::iterator it = input_storages.begin(); it != input_storages.end(); it++) {
        if (*it == storage) {
            input_storages.erase(it);
            break;
        }
    }
}
