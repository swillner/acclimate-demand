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

#include "BusinessConnection.h"

#include "EconomicAgent.h"
#include "Infrastructure.h"
#include "Model.h"
#include "OutputStorage.h"
#include "ProductionSite.h"
#include "PurchasingManager.h"
#include "Region.h"
#include "SalesManager.h"
#include "Sector.h"
#include "Storage.h"
#include "TransportChainLink.h"

BusinessConnection::BusinessConnection(PurchasingManager* buyer, SalesManager* seller, Flow initial_flow__Z_star)
    : buyer(buyer), seller(seller), initial_flow__Z_star(initial_flow__Z_star) {
    seller->business_connections.push_back(this);
    Path path;
    buyer->storage->economic_agent->region->find_path_to(seller->output_storage->economic_agent->region, &path);
    TransportDelay transport_delay__tau = path.distance;

    last_demand_request__D = initial_flow__Z_star;
    last_shipment__Z = initial_flow__Z_star;
    first_transport_link = new TransportChainLink(this, 0, transport_delay__tau, initial_flow__Z_star);
    if (path.infrastructures.size() == 1) {
        path.infrastructures[0]->transport_chain_links.push_back(first_transport_link);
    }
    demand_fulfill_history = 1;
    domestic = (buyer->storage->economic_agent->region == seller->output_storage->economic_agent->region);
}

void BusinessConnection::disconnect_from_geography() {
    Path path;
    buyer->storage->economic_agent->region->find_path_to(seller->output_storage->economic_agent->region, &path);
    if (path.infrastructures.size() == 1) {
        path.infrastructures[0]->remove_transport_chain_link(first_transport_link);
    } else {
        // not implemented yet
    }
}

BusinessConnection::~BusinessConnection() {
    if (seller) {
        disconnect_from_geography();
    }
    delete first_transport_link;
}

void BusinessConnection::push_flow__Z(Flow flow__Z) {
    last_shipment__Z = flow__Z;
    first_transport_link->push_flow__Z(flow__Z);
    if (!domestic && seller && flow__Z > 0) {
        seller->output_storage->economic_agent->region->add_export__Z(flow__Z);
    }
}

TransportDelay BusinessConnection::calc_transport_delay__tau() {
    TransportChainLink* link = first_transport_link;
    TransportDelay res = 0;
    while (link) {
        res += link->current_transport_delay__tau;
        if (link->is_last_link) {
            break;
        } else {
            link = link->next_transport_chain_link;
        }
    }
    return res;
}

void BusinessConnection::deliver_flow__Z(Flow flow__Z) {
    buyer->storage->push_flow__Z(flow__Z);
    if (!domestic) {
        buyer->storage->economic_agent->region->add_import__Z(flow__Z);
    }
}

void BusinessConnection::send_demand_request__D(Demand demand_request__D) {
    last_demand_request__D = demand_request__D;
    seller->add_demand_request__D(demand_request__D);
}

Flow BusinessConnection::calc_transport_flow_deficit() {
    TransportChainLink* link = first_transport_link;
    Flow res = 0;
    while (link) {
        res = res + ((link->initial_transport_delay__tau - 1) * initial_flow__Z_star - link->get_total_flow());
        if (link->is_last_link) {
            break;
        } else {
            link = link->next_transport_chain_link;
        }
    }
    return MAKE_FLOW(res);
}

Flow BusinessConnection::calc_transport_flow() {
    TransportChainLink* link = first_transport_link;
    Flow res = 0;
    while (link) {
        res += link->get_total_flow();
        if (link->is_last_link) {
            break;
        } else {
            link = link->next_transport_chain_link;
        }
    }
    return MAKE_FLOW(res);
}

void BusinessConnection::calc_demand_fulfill_history() {
    // Checks if and in how far demand was fulfilled by these suppliers
    Flow demand_fulfill;
    if (last_demand_request__D > 0) {
        demand_fulfill = last_shipment__Z / last_demand_request__D;
    } else {
        demand_fulfill = 0;
    }

    demand_fulfill_history = seller->output_storage->sector->model->options.history_weight * demand_fulfill_history
                             + (1 - seller->output_storage->sector->model->options.history_weight) * demand_fulfill;
}
