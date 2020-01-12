#!/bin/bash
# This is all very much dev shit, I have just about 0 interest in configuring a server right now
# Hack my bookshelf I guess

# Start the python dev server
./backend/bookshelf.py &
BACKEND_PID=$!
cd frontend && sudo ng serve --host=0.0.0.0 --port=80 --disable-host-check
kill $BACKEND_PID
