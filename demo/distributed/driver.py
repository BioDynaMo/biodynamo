#!/usr/bin/env python
# -----------------------------------------------------------------------------
#
# Copyright (C) The BioDynaMo Project.
# All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

import argparse
import collections
import ctypes
import hashlib
import json
import logging
import os
import re
import sys
import time

import ray
import pyarrow
from pyarrow import plasma  # Can only do this after importing ray.


SIMULATION_START_MARKER = b'a' * 20
SIMULATION_END_MARKER = b'b' * 20
SIMULATION_ID = None

ARGS = None


def hash(*args):
    h = hashlib.sha256()
    for item in args:
        if isinstance(item, str):
            h.update(item)
        if isinstance(item, int):
            h.update(struct.pack('<Q', item))
    return h.digest()[-20:]


def get_node_info():
    node_ip = ray.worker.services.get_node_ip_address()
    info = ray.global_state.client_table()[node_ip]
    logging.debug('Node info: %s', info)
    for d in info:
        client_type = d.get('ClientType')
        if client_type == 'local_scheduler':
            local_scheduler_socket_name = d['LocalSchedulerSocketName']
        elif client_type == 'plasma_manager':
            store_socket_name = d['store_socket_name']
            manager_socket_name = d['manager_socket_name']
    return local_scheduler_socket_name, store_socket_name, manager_socket_name


def load_bdm_library(path=None):
    if path is None:
        path = ARGS.library
    dll = ctypes.CDLL(path)
    try:
        dll.bdm_setup_ray
    except AttributeError:
        raise RuntimeError('There must be a function bdm_setup_ray in the library.')
    if ARGS.mode == 'ray':
      scheduler, store, manager = get_node_info()
      global SIMULATION_ID
      dll.bdm_setup_ray(scheduler, store, manager, SIMULATION_ID, ARGS.partitioning_scheme)
    return dll


@ray.remote
def simulate(library, argv):
    # First create a simulation graph.
    build_simulation_graph.remote()
    # Then kick start it.
    dll = load_bdm_library(library)
    main_argv = (ctypes.c_char_p * len(argv))()
    main_argv[:] = argv
    print('Calling main')
    dll.main(len(argv), main_argv)  # This blocks until the end of the simulation.
    print('Main completed')


@ray.remote
def wait_for_start_signal():
    print('Waiting for start signal')
    start_key = plasma.ObjectID(hash(SIMULATION_ID, SIMULATION_START_MARKER))
    ray.worker.global_worker.plasma_client.fetch([start_key])
    [json_blob] = ray.worker.global_worker.plasma_client.get_buffers(
            [start_key])
    print('Got start signal')
    logging.debug('Received JSON: %s', json_blob.to_pybytes())
    info = json.loads(json_blob.to_pybytes())
    num_steps = info['steps']
    print('C++ says it has received {} steps'.format(num_steps))
    return num_steps, info['bounding_box']


@ray.remote
def partition(num_boxes):
    # We are not actually doing any work here.
    # We only return dummy Python objects so that Ray
    # can build up a dependency graph.
    return [None] * (27 * num_boxes)


# At the end of each simulation step, there will be 27 regions stored in Plasma.
@ray.remote(num_return_vals=27, max_calls=4)
def simulation_step(step_num, node_id, last_iter, bounding_box, *dependencies):
    print('step', step_num, node_id)
    dll = load_bdm_library()
    dll.bdm_simulate_step(step_num, node_id, last_iter,
                          *[ctypes.c_double(x) for x in bounding_box])
    return [None] * 27


NeighborSurface = collections.namedtuple('NeighborSurface', 'box surface')


LEFT = 1
FRONT = 2
BOTTOM = 4
RIGHT = 8
BACK = 16
TOP = 32
# This needs to match C++, but we're using dummy values so that's not needed now.
SURFACE_TO_INDEX = {
        0: 0,
        LEFT: 1,
        LEFT | FRONT: 2,
        LEFT | FRONT | TOP: 3,
        LEFT | FRONT | BOTTOM: 4,
        LEFT | BACK: 5,
        LEFT | BACK | TOP: 6,
        LEFT | BACK | BOTTOM: 7,
        LEFT | TOP: 8,
        LEFT | BOTTOM: 9,
        RIGHT: 10,
        RIGHT | FRONT: 11,
        RIGHT | FRONT | TOP: 12,
        RIGHT | FRONT | BOTTOM: 13,
        RIGHT | BACK: 14,
        RIGHT | BACK | TOP : 15,
        RIGHT | BACK | BOTTOM: 16,
        RIGHT | TOP: 17,
        RIGHT | BOTTOM: 18,
        FRONT: 19,
        FRONT | TOP: 20,
        FRONT | BOTTOM: 21,
        BACK: 22,
        BACK | TOP: 23,
        BACK | BOTTOM: 24,
        TOP: 25,
        BOTTOM: 26,
}


@ray.remote
def build_simulation_graph():
    dll = load_bdm_library()
    num_boxes = dll.bdm_get_box_count()
    print('C++ says there are {} boxes'.format(num_boxes))

    # Build a neighbor map. Each box will have a list of neighbor surfaces.
    neighbor_map = collections.defaultdict(list)  # type Dict[int, List[NeighborSurface]]
    # Each box has at most 26 neighbors, each is a 2-int (box id, surface).
    neighbors = (ctypes.c_int * (26 * 2))()
    for box in xrange(num_boxes):
        neighbor_count = dll.bdm_get_neighbor_surfaces(box, neighbors)
        for i in range(neighbor_count):
            neighbor_map[box].append(NeighborSurface(
                    neighbors[i * 2], SURFACE_TO_INDEX[neighbors[i * 2 + 1]]))
        box += 1

    partitions = partition._submit(args=[num_boxes], num_return_vals=27 * num_boxes)
    last_step = {box: partitions[box * 27 : box * 27 + 27] for box in xrange(num_boxes)}
    this_step = collections.defaultdict(list)

    num_steps, bounding_box = ray.get(wait_for_start_signal.remote())
    for step in xrange(num_steps):
        for box in xrange(num_boxes):
            deps = [last_step[x.box][x.surface] for x in neighbor_map[box]]
            this_step[box] = simulation_step.remote(
                    step, box, step == (num_steps - 1), bounding_box, *deps)
        last_step, this_step = this_step, last_step

    ray.get(send_end_signal.remote(*[last_step[i][0] for i in neighbor_map]))


@ray.remote
def send_end_signal(*dependencies):
    ray.worker.global_worker.put_object(ray.ObjectID(
        hash(SIMULATION_ID, SIMULATION_END_MARKER)), '')


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument('-l', '--library', default=None, required=True,
                        help='The path to the compiled dynamic library of the simulation.')
    parser.add_argument('-m', '--mode', choices=['ray', 'local'], default='ray',
                        help='Whether the simulation will be run locally, or with Ray.')
    parser.add_argument('-r', '--redis-address', default=None,
                        help='The ip:port address of Redis server if this is a part of a cluster. '
                             'If this is not specified, and the mode is "ray", a new Ray cluster '
                             'will be started.')
    parser.add_argument('-p', '--partitioning_scheme', default='2-1-1',
                        help='The partitioning scheme. 2-1-1 partitions 2 boxes along the x-axis. '
                             '3-3-3 partitions 27 boxes along x-, y-, and z- axis. This argument '
                             'is only effective in Ray mode.')
    for i in range(1, len(sys.argv)):
        arg = sys.argv[i]
        if arg == '--':
            args = parser.parse_args(sys.argv[1:i])
            unknowns = sys.argv[i + 1:]
    else:
        args, unknowns = parser.parse_known_args()
    global ARGS
    ARGS = args
    if ARGS.mode == 'ray':
        run_with_ray(args.library, args.redis_address, unknowns)
    else:
        run_normally(args.library, unknowns)


def run_normally(library, argv):
    dll = load_bdm_library(library)
    main_argv = (ctypes.c_char_p * (len(argv) + 1))()
    main_argv[0] = library
    main_argv[1:] = argv
    dll.main(len(main_argv), main_argv)  # This blocks until the end of the simulation.


def check_partitioning_scheme():
    if re.match(r'^\d+-\d+-\d+$', ARGS.partitioning_scheme) is None:
        raise ValueError('Invalid partitioning scheme.')


def run_with_ray(source_library, redis_address, argv):
    check_partitioning_scheme()
    global SIMULATION_ID
    SIMULATION_ID = ray.utils.random_string()
    address_info = ray.init(redis_address=redis_address)
    logging.debug(address_info)
    sim_name = os.path.basename(source_library).lstrip('lib').rstrip('.so')
    ray.get(simulate.remote(os.path.abspath(source_library), [sim_name] + argv))
    print('Simulation completed successfully!')


if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG)
    main(sys.argv)
