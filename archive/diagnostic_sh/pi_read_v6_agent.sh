echo "PID: $(cat /tmp/microros_agent_v6.pid)"
ps -p "$(cat /tmp/microros_agent_v6.pid)" -o pid=,stat=,cmd= || true
echo ---
sed -n '1,220p' /tmp/microros_agent_v6.log
