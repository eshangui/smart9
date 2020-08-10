SERVERNAME="d9main"

start()
{
    echo "start $SERVERNAME"
    /home/meirilin/smart9/$SERVERNAME
    echo "start $SERVERNAME ok!"
    exit 0;
}

stop()
{
    echo "stop $SERVERNAME"
    killall $SERVERNAME
    echo "stop $SERVERNAME ok!"
}

case "$1" in
start)
    start
    ;;
stop)
    stop
    ;;
restart)
    stop
    start
    ;;
*)
    echo "usage: $0 start|stop|restart"
    exit 0;
esac
exit