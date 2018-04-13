#!/bin/bash
# DESCRIPTION:
#   Lists all archived branches. Synchronizes with origin first
# USAGE:
#   list.sh
# EXAMPLE OUTPUT:
#   088d1..(shortened)..6a commit	refs/archive/lukas/commutative-pair-operation
#
#   archive branch name does not include 'refs/archive'
#   -> archived branch name is 'lukas/commutative-pair-operation'

# download all references
git fetch origin +refs/archive/*:refs/archive/*

git for-each-ref refs/archive
