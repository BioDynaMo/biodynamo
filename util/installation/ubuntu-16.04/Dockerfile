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
RUN apt-get update && apt-get install -y \
  git \
  sudo \
  lsb-release \
  lsb-core \
  man \
  software-properties-common \
  wget \
  xvfb

RUN git config --system user.name "Test User" && \
    git config --system user.email user@test.com

# update user id and group id such that mapped volumes can be accessed with the
# same rights as on the host. Files created by the container can also be
# accessed on the host without chowning.
ARG HOST_UID
ARG HOST_GID

RUN groupadd -g $HOST_GID testuser && \
    useradd -u $HOST_UID -g $HOST_GID -m -c "Testuser" testuser && \
    passwd --delete testuser && \
    echo "testuser ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

USER testuser

# preserve the environment variable BDM_LOCAL_LFS when calling scripts with
# sudo
RUN echo "Defaults env_keep += \"BDM_LOCAL_LFS\"" | sudo tee -a /etc/sudoers
