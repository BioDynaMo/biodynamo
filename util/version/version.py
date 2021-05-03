# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & Newcastle University for the benefit of the
# BioDynaMo collaboration. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
#
# See the LICENSE file distributed with this work for details.
# See the NOTICE file distributed with this work for additional information
# regarding copyright ownership.
#
# -----------------------------------------------------------------------------

class Version:
    @staticmethod
    def string():
        return "@VERSION@"

    @staticmethod
    def shortstring():
        # python doesn't allow leading zeros in decimal numbers
        # -> use string to int conversion
        major = int('@VERSION_MAJOR@')
        minor = int('@VERSION_MINOR@')
        patch = int('@VERSION_PATCH@')
        if patch == 0:
            return "{}.{}".format(major, minor)
        else:
            return "{}.{}.{}".format(major, minor, patch)

