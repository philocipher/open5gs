#!/bin/bash

(sudo lsof -t -i :7777 -i :8805 -i :38412) | xargs -r sudo kill -9 || echo "No process is using port 7777 or 8805 or 38412."