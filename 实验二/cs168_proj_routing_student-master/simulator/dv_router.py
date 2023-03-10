"""
Your awesome Distance Vector router for CS 168

Based on skeleton code by:
  MurphyMc, zhangwen0411, lab352
"""

import sim.api as api
from cs168.dv import RoutePacket, \
                     Table, TableEntry, \
                     DVRouterBase, Ports, \
                     FOREVER, INFINITY


class DVRouter(DVRouterBase):

    # A route should time out after this interval
    ROUTE_TTL = 15

    # Dead entries should time out after this interval
    GARBAGE_TTL = 10

    # -----------------------------------------------
    # At most one of these should ever be on at once
    SPLIT_HORIZON = False
    POISON_REVERSE = True
    # -----------------------------------------------

    # Determines if you send poison for expired routes
    POISON_EXPIRED = False

    # Determines if you send updates when a link comes up
    SEND_ON_LINK_UP = False

    # Determines if you send poison when a link goes down
    POISON_ON_LINK_DOWN = False

    def __init__(self):
        """
        Called when the instance is initialized.
        DO NOT remove any existing code from this method.
        However, feel free to add to it for memory purposes in the final stage!
        """
        assert not (self.SPLIT_HORIZON and self.POISON_REVERSE), \
                    "Split horizon and poison reverse can't both be on"

        self.start_timer()  # Starts signaling the timer at correct rate.

        # Contains all current ports and their latencies.
        # See the write-up for documentation.
        self.ports = Ports()

        # This is the table that contains all current routes
        self.table = Table()
        self.table.owner = self

    def add_static_route(self, host, port):
        assert port in self.ports.get_all_ports(), "Link should be up, but is not."
        self.table[host] = TableEntry(dst = host, port = port, latency = self.ports.get_latency(port), expire_time = FOREVER)

    def handle_data_packet(self, packet, in_port):
        dst = packet.dst
        if (dst in self.table and self.table[dst].latency < INFINITY):
                self.send(packet, self.table[dst].port)

    def send_routes(self, force=False, single_port=None):
        if (not self.SPLIT_HORIZON and not self.POISON_REVERSE):
            for port in self.ports.link_to_lat.keys():
                for host, entry in self.table.items():
                    self.send_route(port, host, entry.latency)
        else:
            for port in self.ports.link_to_lat.keys():
                for host, entry in self.table.items():
                    if (entry.port != port):
                        self.send_route(port, host, entry.latency)
                    else:
                        if(self.POISON_REVERSE):
                            self.send_route(port, host, INFINITY)

    def expire_routes(self):
        expire_list = []
        for host,entry in self.table.items():
            if (api.current_time() >= entry.expire_time):
                expire_list.append(host)
        for host in expire_list:
            del(self.table[host])

    def handle_route_advertisement(self, route_dst, route_latency, port):
        
        if (route_dst not in self.table): 
            self.table[route_dst] = TableEntry(dst = route_dst, port = port, latency = route_latency + self.ports.get_latency(port), expire_time = api.current_time() + self.ROUTE_TTL)
        else:
            if (port == self.table[route_dst].port or self.table[route_dst].latency > route_latency + self.ports.get_latency(port)):
                self.table[route_dst] = TableEntry(dst = route_dst, port = port, latency = route_latency + self.ports.get_latency(port), expire_time = api.current_time() + self.ROUTE_TTL)
        '''
        if (route_dst not in self.table):
            if(route_latency < INFINITY): 
                self.table[route_dst] = TableEntry(dst = route_dst, port = port, latency = route_latency + self.ports.get_latency(port), expire_time = api.current_time() + self.ROUTE_TTL)
        else:
            if(route_latency >= INFINITY):
                if(self.table[route_dst].port == port):
                    self.table[route_dst].latency = INFINITY
            else:
                if (port == self.table[route_dst].port or self.table[route_dst].latency > route_latency + self.ports.get_latency(port)):
                    if(self.table[route_dst].latency < INFINITY):
                        self.table[route_dst] = TableEntry(dst = route_dst, port = port, latency = route_latency + self.ports.get_latency(port), expire_time = api.current_time() + self.ROUTE_TTL)
                    else:
                        expt = self.table[route_dst].expire_time
                        self.table[route_dst] = TableEntry(dst = route_dst, port = port, latency = route_latency + self.ports.get_latency(port), expire_time = expt)
        '''
        
    def handle_link_up(self, port, latency):
        """
        Called by the framework when a link attached to this router goes up.

        :param port: the port that the link is attached to.
        :param latency: the link latency.
        :returns: nothing.
        """
        self.ports.add_port(port, latency)

        # TODO: fill in the rest!

    def handle_link_down(self, port):
        """
        Called by the framework when a link attached to this router does down.

        :param port: the port number used by the link.
        :returns: nothing.
        """
        self.ports.remove_port(port)

        # TODO: fill this in!

    # Feel free to add any helper methods!
