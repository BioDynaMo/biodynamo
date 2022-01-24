# -----------------------------------------------------------------------------
#
# Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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


class Print:
    PURPLE = "\033[95m"
    CYAN = "\033[96m"
    DARKCYAN = "\033[36m"
    BLUE = "\033[94m"
    GREEN = "\033[92m"
    YELLOW = "\033[93m"
    RED = "\033[91m"
    BOLD = "\033[1m"
    UNDERLINE = "\033[4m"
    END = "\033[0m"

    @staticmethod
    def success(message):
        print(Print.BOLD + Print.GREEN + str(message) + Print.END)

    @staticmethod
    def error(message):
        print(Print.RED + str(message) + Print.END)

    @staticmethod
    def warning(message):
        print(Print.YELLOW + str(message) + Print.END)

    @staticmethod
    def new_step(message):
        print("\n" + Print.BOLD + Print.BLUE + str(message) + Print.END)

    @staticmethod
    def new_step_in_config(message1, message2):
        print(
            "\n"
            + Print.BOLD
            + Print.GREEN
            + str(message1)
            + Print.END
            + " ("
            + str(message2)
            + ")"
        )
