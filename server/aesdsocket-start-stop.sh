#!/bin/sh

# Set the path to your aesdsocket executable
AESDSOCKET_BIN="/path/to/your/aesdsocket"

case "$1" in
  start)
    echo "Starting aesdsocket daemon..."
    start-stop-daemon --start --background --make-pidfile --pidfile /var/run/aesdsocket.pid \
      --exec "$AESDSOCKET_BIN" -d
    ;;
  stop)
    echo "Stopping aesdsocket daemon..."
    start-stop-daemon --stop --pidfile /var/run/aesdsocket.pid
    ;;
  restart)
    $0 stop
    sleep 1
    $0 start
    ;;
  *)
    echo "Usage: $0 {start|stop|restart}"
    exit 1
    ;;
esac

exit 0
