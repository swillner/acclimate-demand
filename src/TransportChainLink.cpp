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

#include "TransportChainLink.h"

#include "BusinessConnection.h"
#include "Infrastructure.h"

TransportChainLink::TransportChainLink(BusinessConnection* business_connection,
                                       TransportChainLink* next_transport_chain_link,
                                       TransportDelay transport_delay__tau,
                                       Flow initial_flow__Z_star)
    : current_transport_delay__tau(transport_delay__tau), initial_transport_delay__tau(transport_delay__tau) {
    if (next_transport_chain_link) {
        is_last_link = false;
        this->next_transport_chain_link = next_transport_chain_link;
    } else {
        is_last_link = true;
        this->business_connection = business_connection;
    }
    if (initial_transport_delay__tau > 1) {
        transport_queue = new Flow[initial_transport_delay__tau - 1];
        for (int i = 0; i < initial_transport_delay__tau - 1; i++) {
            transport_queue[i] = initial_flow__Z_star;
        }
    } else {
        transport_queue = 0;
    }
    pos = 0;
}

TransportChainLink::~TransportChainLink() {
    if (transport_queue) {
        delete[] transport_queue;
    }
    if (!is_last_link) {
        delete next_transport_chain_link;
    }
}

void TransportChainLink::push_flow__Z(Flow flow__Z) {
    if (current_transport_delay__tau > 1) {
        Flow front_flow__Z = transport_queue[pos];
        transport_queue[pos] = flow__Z;
        pos = (pos + 1) % (current_transport_delay__tau - 1);
        if (is_last_link) {
            business_connection->deliver_flow__Z(front_flow__Z);
        } else {
            next_transport_chain_link->push_flow__Z(front_flow__Z);
        }
    } else {
        if (is_last_link) {
            business_connection->deliver_flow__Z(flow__Z);
        } else {
            next_transport_chain_link->push_flow__Z(flow__Z);
        }
    }
}

void TransportChainLink::set_forcing__nu(Forcing forcing__nu) {
    // not implemented yet
}

Flow TransportChainLink::get_total_flow() {
    if (current_transport_delay__tau > 1) {
        Flow res = 0;
        for (int i = 0; i < current_transport_delay__tau - 1; i++) {
            res += transport_queue[i];
        }
        return res;
    } else {
        return 0;
    }
}
