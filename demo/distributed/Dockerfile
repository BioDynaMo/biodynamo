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

FROM ubuntu:16.04

# lsb-release and lsb-core required to determine operating system
# man required by ROOT
RUN apt-get update && \
    apt-get install -y sudo python-pip psmisc net-tools \
        libxt-dev libgl-dev libx11-dev libxpm4

# update user id and group id such that mapped volumes can be accessed with the
# same rights as on the host. Files created by the container can also be
# accessed on the host without chowning.
ARG USER
ARG HOST_UID
ARG HOST_GID

RUN groupadd -g $HOST_GID $USER && \
    useradd -u $HOST_UID -g $HOST_GID -m -c "Testuser" $USER && \
    passwd --delete $USER && \
    echo "$USER ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

USER $USER
