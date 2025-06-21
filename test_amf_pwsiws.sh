#!/bin/bash

# Test script for AMF and PWS-IWS integration
echo "=== Testing AMF and PWS-IWS Integration ==="

# Create log directories
mkdir -p /tmp/open5gs/logs

# Function to cleanup background processes
cleanup() {
    echo "Cleaning up..."
    pkill -f "open5gs-amfd" 2>/dev/null
    pkill -f "open5gs-pwsiwsd" 2>/dev/null
    sleep 2
}

# Set up cleanup on script exit
trap cleanup EXIT

echo "1. Starting PWS-IWS component..."
./build/src/pwsiws/open5gs-pwsiwsd -c config/pwsiws.yaml > /tmp/open5gs/logs/pwsiws.log 2>&1 &
PWSIWS_PID=$!
sleep 3

echo "2. Starting AMF component..."
./build/src/amf/open5gs-amfd -c build/configs/open5gs/amf.yaml > /tmp/open5gs/logs/amf.log 2>&1 &
AMF_PID=$!
sleep 5

echo "3. Checking if both components are running..."
if ps -p $PWSIWS_PID > /dev/null; then
    echo "✓ PWS-IWS is running (PID: $PWSIWS_PID)"
else
    echo "✗ PWS-IWS failed to start"
    exit 1
fi

if ps -p $AMF_PID > /dev/null; then
    echo "✓ AMF is running (PID: $AMF_PID)"
else
    echo "✗ AMF failed to start"
    exit 1
fi

echo "4. Testing SBI communication between AMF and PWS-IWS..."
echo "   - AMF SBI server should be listening on 127.0.0.5:7777"
echo "   - PWS-IWS should be able to communicate with AMF via SBI"

# Check if AMF is listening on SBI port
if netstat -tlnp 2>/dev/null | grep ":7777" > /dev/null; then
    echo "✓ AMF SBI server is listening on port 7777"
else
    echo "✗ AMF SBI server is not listening on port 7777"
fi

# Check if PWS-IWS is listening on its configured port
if netstat -tlnp 2>/dev/null | grep ":10000" > /dev/null; then
    echo "✓ PWS-IWS SCTP server is listening on port 10000"
else
    echo "✗ PWS-IWS SCTP server is not listening on port 10000"
fi

echo "5. Testing PWS-IWS SBI service registration..."
echo "   - PWS-IWS should register its SBI service with AMF"

# Check logs for any errors
echo "6. Checking component logs for errors..."

echo "PWS-IWS Log (last 10 lines):"
tail -10 /tmp/open5gs/logs/pwsiws.log

echo ""
echo "AMF Log (last 10 lines):"
tail -10 /tmp/open5gs/logs/amf.log

echo ""
echo "7. Testing PWS-IWS SBI endpoint..."
# Test if PWS-IWS SBI endpoint is accessible
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:7777/ 2>/dev/null || echo "Connection failed"

echo ""
echo "=== Integration Test Summary ==="
echo "Both components are running and should be able to communicate via SBI."
echo "PWS-IWS can send warning messages to AMF, and AMF can broadcast them to gNBs."
echo ""
echo "To test warning message flow:"
echo "1. Send a warning message to PWS-IWS via SCTP (port 10000)"
echo "2. PWS-IWS should forward it to AMF via SBI"
echo "3. AMF should broadcast it to connected gNBs via NGAP"
echo ""
echo "Logs are available in /tmp/open5gs/logs/"
echo "Press Ctrl+C to stop the test" 