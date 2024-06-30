#!/bin/bash

valgrind  --tool=memcheck --leak-check=full --show-reachable=yes ./gtester "$@"
