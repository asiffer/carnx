#!/bin/bash
#
# Here we assume that carnxd is listening to /run/carnx.sock
#

GRPCURL=$(command -v grpcurl)
GRPCURL_FLAGS="-plaintext -emit-defaults -unix -import-path api/ -proto carnx.proto"
GRPCURL_ADDR="/run/carnx.sock"

title() {
    title="$1"
    title_size=${#title}
    terminal_width=$(tput cols)
    fill=$((terminal_width - title_size - 5))
    end=$(printf "%0${fill}d" 0 | tr '0' '=')
    echo -e "\n=== \033[0;34m$1\033[0m $end"
}

result() {
    echo -n "Check: "
    if [ "$1" == "$2" ]; then
        echo -e "[\033[0;32mOK\033[0m]"
    else
        echo -e "[\033[0;31mERROR\033[0m]: got $1, expected $2"
    fi
}

# gRPC testing function
test="${GRPCURL} ${GRPCURL_FLAGS}"

# # start the service
# systemctl start carnx.service

title "Getting the number of counters"
${test} ${GRPCURL_ADDR} "api.Carnx/GetNbCounters"

title "Getting counter names"
${test} ${GRPCURL_ADDR} "api.Carnx/GetCounterNames"

title "Getting each counter value"
# save the number of counters
n=$(${test} ${GRPCURL_ADDR} "api.Carnx/GetNbCounters" | jq ".nbCounters")

for ((i = 0; i <= n; i++)); do
    data=$(printf '{"id":%s}' $i)
    ${test} -d "${data}" ${GRPCURL_ADDR} "api.Carnx/GetCounter"
done

title "Getting each counter value through their name"
# save the number of counters
list="$(${test} ${GRPCURL_ADDR} "api.Carnx/GetCounterNames" | jq ".counters" | tr -d '][,')"

for ctr in $list; do
    echo "$ctr"
    data=$(printf '{"name":%s}' "${ctr}")
    ${test} -d "${data}" ${GRPCURL_ADDR} "api.Carnx/GetCounterByName"
done

title "Performing a snapshot"
${test} ${GRPCURL_ADDR} "api.Carnx/Snapshot"

title "Detaching"
${test} ${GRPCURL_ADDR} "api.Carnx/Detach"

attached="$(${test} ${GRPCURL_ADDR} "api.Carnx/IsAttached" | jq ".attached")"
result "${attached}" "false"

title "Attaching to 'lo'"
${test} -d '{"interface": "lo", "xdp_flags": 0}' ${GRPCURL_ADDR} "api.Carnx/Attach"

attached="$(${test} ${GRPCURL_ADDR} "api.Carnx/IsAttached" | jq ".attached")"
result "${attached}" "true"

title "Detaching and unloading"
${test} ${GRPCURL_ADDR} "api.Carnx/Detach"

attached="$(${test} ${GRPCURL_ADDR} "api.Carnx/IsAttached" | jq ".attached")"
result "${attached}" "false"

${test} ${GRPCURL_ADDR} "api.Carnx/Unload"

loaded="$(${test} ${GRPCURL_ADDR} "api.Carnx/IsLoaded" | jq ".loaded")"
result "${loaded}" "false"

title "Load and attach to 'lo'"
${test} -d '{"interface": "lo", "xdp_flags": 0, "bpf_program": "./bin/carnx.bpf"}' ${GRPCURL_ADDR} "api.Carnx/LoadAndAttach"

loaded="$(${test} ${GRPCURL_ADDR} "api.Carnx/IsLoaded" | jq ".loaded")"
result "${loaded}" "true"

title "Final snapshots"
for ((i = 0; i <= 5; i++)); do
    ${test} ${GRPCURL_ADDR} "api.Carnx/Snapshot"
    sleep 0.5
done
