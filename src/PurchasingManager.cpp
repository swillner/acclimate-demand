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

#include "PurchasingManager.h"

#include <iostream>
#include "BusinessConnection.h"
#include "CapacityManager.h"
#include "FinalDemand.h"
#include "Model.h"
#include "OutputStorage.h"
#include "ProductionSite.h"
#include "SalesManager.h"
#include "Sector.h"
#include "Storage.h"

PurchasingManager::PurchasingManager() {
    storage = 0;
    demand__D = 0;
}

PurchasingManager::~PurchasingManager() {
    for (std::vector<BusinessConnection*>::iterator it = business_connections.begin(); it != business_connections.end(); it++) {
        (*it)->seller->remove_business_connection(*it);
        delete *it;
    }
}

void PurchasingManager::iterate_consumption_and_production() {}

void PurchasingManager::iterate_demand() {
    calc_demand__D();
    send_demand_requests__D();
}

void PurchasingManager::calc_demand__D() {
    demand__D = storage->desired_used_flow__U_tilde
                + (storage->initial_content__S_star - storage->content__S + storage->initial_input_flow__I_star - storage->next_input_flow__I()
                   + calc_transport_flow_deficit())
                      / storage->sector->storage_refill_enforcement__gamma;
    demand__D = MAX((Demand)0, MAKE_DEMAND(demand__D));
}

void PurchasingManager::remove_business_connection(BusinessConnection* business_connection) {
    for (std::vector<BusinessConnection*>::iterator it = business_connections.begin(); it != business_connections.end(); it++) {
        if (*it == business_connection) {
            storage->add_initial_flow__Z_star(-business_connection->initial_flow__Z_star);
            business_connections.erase(it);
            break;
        }
    }
}

Flow PurchasingManager::calc_transport_flow_deficit() {
    Flow res = 0;
    for (auto b = business_connections.begin(); b != business_connections.end(); b++) {
        res += (*b)->calc_transport_flow_deficit();
    }
    return res;
}

Flow PurchasingManager::calc_transport_flow() {
    Flow res = 0;
    for (auto b = business_connections.begin(); b != business_connections.end(); b++) {
        res += (*b)->calc_transport_flow();
    }
    return res;
}

Flow PurchasingManager::sum_last_shipments() {
    Flow res = 0;
    for (auto b = business_connections.begin(); b != business_connections.end(); b++) {
        res += (*b)->last_shipment__Z;
    }
    return res;
}
