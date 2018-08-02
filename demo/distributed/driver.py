#!/usr/bin/env python

import argparse
import ctypes
import hashlib
import logging
import os
import json
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
    dll.main(len(argv), main_argv)  # This blocks until the end of the simulation.


@ray.remote
def wait_for_start_signal():
    start_key = plasma.ObjectID(hash(SIMULATION_ID, SIMULATION_START_MARKER))
    ray.worker.global_worker.plasma_client.fetch([start_key])
    [json_blob] = ray.worker.global_worker.plasma_client.get_buffers(
            [start_key])
    logging.debug('Received JSON: %s', json_blob.to_pybytes())
    info = json.loads(json_blob.to_pybytes())
    num_steps = info['steps']
    print('C++ says it has received {} steps'.format(num_steps))
    return num_steps, info['bounding_box']


@ray.remote
def partition(num_nodes=2):
    # We are not actually doing any work here.
    # We only return dummy Python objects so that Ray
    # can build up a dependency graph.
    return [None] * 19 * num_nodes


@ray.remote(num_return_vals=19)
def simulation_step(step_num, node_id, last_iter, bounding_box, *dependencies):
    print('step', step_num, node_id)
    dll = load_bdm_library()
    dll.simulate_step(step_num, node_id, last_iter,
                      *[ctypes.c_double(x) for x in bounding_box])
    return [None] * 19


@ray.remote
def build_simulation_graph():
    num_nodes = 2
    partitions = partition._submit(args=[num_nodes], num_return_vals=19 * num_nodes)
    node_0 = partitions[:19]
    node_1 = partitions[19:]
    num_steps, bounding_box = ray.get(wait_for_start_signal.remote())
    for step in range(num_steps):
        node_0 = simulation_step.remote(step, 0, step == num_steps - 1, bounding_box, node_0[0])
        node_1 = simulation_step.remote(step, 1, step == num_steps - 1, bounding_box, node_1[0])
    ray.get(node_0)
    ray.get(node_1)
    ray.get(send_end_signal.remote())


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
    parser.add_argument('-p', '--partitioning_scheme', choices=['2-1-1', '3-3-3'], default='2-1-1',
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


def run_with_ray(source_library, redis_address, argv):
    global SIMULATION_ID
    SIMULATION_ID = ray.utils.random_string()
    address_info = ray.init(redis_address=redis_address)
    logging.debug(address_info)
    sim_name = os.path.basename(source_library).lstrip('lib').rstrip('.so')
    ray.get(simulate.remote(os.path.abspath(source_library), [sim_name] + argv))


if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG)
    main(sys.argv)
