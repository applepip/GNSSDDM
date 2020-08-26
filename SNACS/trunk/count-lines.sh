#!/bin/bash

cat $(find . -type f -iregex ".*.[cpp,h]") | wc -l
