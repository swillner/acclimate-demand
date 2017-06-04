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

#include "SalesManager.h"

#include "BusinessConnection.h"
#include "Model.h"
#include "OutputStorage.h"
#include "ProductionSite.h"
#include "PurchasingManager.h"
#include "Sector.h"

SalesManager::SalesManager() {
    sum_demand_requests__D = 0;
    output_storage = 0;
    demand_forcing = -1;
}

SalesManager::~SalesManager() {
    for (std::vector<BusinessConnection*>::iterator it = business_connections.begin(); it != business_connections.end(); it++) {
        PurchasingManager* buyer = (*it)->buyer;
        buyer->remove_business_connection(*it);
        delete *it;
        if (buyer->business_connections.size() == 0
            && !(buyer->storage->economic_agent->type == PRODUCTION_SITE
                 && (void*)buyer->storage == (void*)((ProductionSite*)buyer->storage->economic_agent)->output_storage)) {
            buyer->storage->economic_agent->remove_storage(buyer->storage);
            delete buyer->storage;
        }
    }
}

void SalesManager::add_demand_request__D(Demand demand_request__D) {
    output_storage->sector->add_demand_request__D(demand_request__D);

    if (demand_forcing != -1) {
        sum_demand_requests__D = demand_forcing;
    } else {
#pragma omp atomic
        sum_demand_requests__D += demand_request__D;
    }
}

void SalesManager::add_initial_demand_request__D_star(Demand initial_demand_request__D_star) { sum_demand_requests__D += initial_demand_request__D_star; }

void SalesManager::iterate_supply() {
    distribute(output_storage->output_flow__chi);
    sum_demand_requests__D = 0;
}

Flow SalesManager::calc_transport_flow() {
    Flow res = 0;
    for (int i = 0; i < business_connections.size(); i++) {
        res += business_connections[i]->calc_transport_flow();
    }
    return res;
}

void SalesManager::remove_business_connection(BusinessConnection* business_connection) {
    for (std::vector<BusinessConnection*>::iterator it = business_connections.begin(); it != business_connections.end(); it++) {
        if (*it == business_connection) {
            ((ProductionSite*)output_storage->economic_agent)->add_initial_production__X_star(-business_connection->initial_flow__Z_star);
            business_connections.erase(it);
            break;
        }
    }
}
