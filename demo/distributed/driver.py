#!/usr/bin/env python

import argparse
import ctypes
import logging
import os
import struct
import sys
import time

import ray
import pyarrow
from pyarrow import plasma  # Can only do this after importing ray.


SIMULATION_START_MARKER = b'a' * 20
SIMULATION_END_MARKER = b'b' * 20


def get_node_info():
    node_ip = ray.worker.services.get_node_ip_address()
    info = ray.global_state.client_table()[node_ip]
    logging.error('Node info: %s', info)
    for d in info:
        client_type = d.get('ClientType')
        if client_type == 'local_scheduler':
            local_scheduler_socket_name = d['LocalSchedulerSocketName']
        elif client_type == 'plasma_manager':
            store_socket_name = d['store_socket_name']
            manager_socket_name = d['manager_socket_name']
    return local_scheduler_socket_name, store_socket_name, manager_socket_name


def load_bdm_library(path):
    dll = ctypes.CDLL(path)
    try:
        dll.bdm_setup_ray
    except AttributeError:
        raise RuntimeError('There must be a function bdm_setup_ray in the library.')
    scheduler, store, manager = get_node_info()
    dll.bdm_setup_ray(scheduler, store, manager)
    return dll


@ray.remote
def initialize_simulation(library, argv):
    dll = load_bdm_library(library)
    main_argv = (ctypes.c_char_p * len(argv))()
    main_argv[:] = argv
    dll.main(len(argv), main_argv)


@ray.remote
def simulate():
    ray.worker.global_worker.plasma_client.fetch(
            [plasma.ObjectID(SIMULATION_START_MARKER)])
    [num_steps_blob] = ray.worker.global_worker.plasma_client.get_buffers(
            [plasma.ObjectID(SIMULATION_START_MARKER)])
    [num_steps] = struct.unpack('<Q', num_steps_blob)
    print('C++ says it has received {} steps'.format(num_steps))
    ray.worker.global_worker.put_object(ray.ObjectID(SIMULATION_END_MARKER), '')
    time.sleep(1)


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
    for i in range(1, len(sys.argv)):
        arg = sys.argv[i]
        if arg == '--':
            args = parser.parse_args(sys.argv[1:i])
            unknowns = sys.argv[i + 1:]
    else:
        args, unknowns = parser.parse_known_args()
    if args.mode == 'ray':
        run_with_ray(args.library, args.redis_address, unknowns)


def run_with_ray(source_library, redis_address, argv):
    address_info = ray.init(redis_address=redis_address)
    logging.debug(address_info)
    sim_name = os.path.basename(source_library).lstrip('lib').rstrip('.so')
    initialize_simulation.remote(os.path.abspath(source_library), [sim_name] + argv)
    ray.get(simulate.remote())


if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG)
    main(sys.argv)
