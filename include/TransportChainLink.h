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

#ifndef _TRANSPORTCHAINLINK_H
#define _TRANSPORTCHAINLINK_H

#include <math.h>

#include "types.h"

class Infrastructure;
class BusinessConnection;

class TransportChainLink {
  protected:
    Flow* transport_queue;
    TransportDelay pos;

  public:
    TransportDelay current_transport_delay__tau;
    TransportDelay const initial_transport_delay__tau;
    bool is_last_link;
    union {
        TransportChainLink* next_transport_chain_link;
        BusinessConnection* business_connection;
    };

  public:
    TransportChainLink(BusinessConnection* business_connection,
                       TransportChainLink* next_transport_chain_link,
                       TransportDelay initial_transport_delay__tau,
                       Flow initial_flow__Z_star);
    ~TransportChainLink();
    void push_flow__Z(Flow flow__Z);
    void set_forcing__nu(Forcing forcing__nu);
    Flow get_total_flow();
};

#endif
