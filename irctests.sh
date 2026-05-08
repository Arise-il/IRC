#!/bin/bash

# ─── CONFIG ──────────────────────────────────────────────────────────────────
HOST="${1:-127.0.0.1}"
PORT="${2:-6667}"
PASSWORD="${3:-password}"

# ─── COLORS ──────────────────────────────────────────────────────────────────
GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[0;33m"
CYAN="\033[0;36m"
BOLD="\033[1m"
RESET="\033[0m"

# ─── COUNTERS ────────────────────────────────────────────────────────────────
PASS_COUNT=0
FAIL_COUNT=0
TOTAL=0

# ─── HELPERS ─────────────────────────────────────────────────────────────────

# Send commands with correct \r\n line endings
irc_send() {
    local timeout=$1
    shift
    # %b interprets \r and \n as actual carriage return and newline
    printf "%b" "$@" | nc -q "$timeout" "$HOST" "$PORT" 2>/dev/null
}

assert_contains() {
    local name="$1"
    local response="$2"
    local expected="$3"
    TOTAL=$((TOTAL + 1))
    if echo "$response" | grep -qE "$expected"; then
        echo -e "  ${GREEN}PASS${RESET} — $name"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        echo -e "  ${RED}FAIL${RESET} — $name"
        echo -e "       ${YELLOW}Expected to find:${RESET} $expected"
        echo -e "       ${YELLOW}Got:${RESET} $(echo "$response" | tr '\r' ' ' | head -5)"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi
}

assert_not_contains() {
    local name="$1"
    local response="$2"
    local unexpected="$3"
    TOTAL=$((TOTAL + 1))
    if echo "$response" | grep -qE "$unexpected"; then
        echo -e "  ${RED}FAIL${RESET} — $name"
        echo -e "       ${YELLOW}Should NOT contain:${RESET} $unexpected"
        echo -e "       ${YELLOW}Got:${RESET} $(echo "$response" | tr '\r' ' ' | head -5)"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    else
        echo -e "  ${GREEN}PASS${RESET} — $name"
        PASS_COUNT=$((PASS_COUNT + 1))
    fi
}

print_section() {
    echo ""
    echo -e "${CYAN}${BOLD}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"
    echo -e "${CYAN}${BOLD}  $1${RESET}"
    echo -e "${CYAN}${BOLD}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"
}

# ─── CHECK SERVER IS RUNNING ─────────────────────────────────────────────────
echo ""
echo -e "${BOLD}  IRC Server Test Suite${RESET}"
echo -e "  Host: $HOST | Port: $PORT | Password: $PASSWORD"
echo ""

nc -z "$HOST" "$PORT" 2>/dev/null
if [ $? -ne 0 ]; then
    echo -e "${RED}${BOLD}  ERROR: Cannot connect to $HOST:$PORT${RESET}"
    echo -e "  Make sure your server is running first."
    echo ""
    exit 1
fi
echo -e "  ${GREEN}Server is reachable${RESET}"

# ─────────────────────────────────────────────────────────────────────────────
print_section "1. REGISTRATION"
# ─────────────────────────────────────────────────────────────────────────────

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK testuser1\r\nUSER testuser1 0 * :Test\r\n")
assert_contains "Correct registration gets 001 welcome" "$R" "001"

R=$(irc_send 2 "PASS wrongpass\r\nNICK testuser2\r\nUSER testuser2 0 * :Test\r\n")
assert_contains "Wrong password gets 464 error" "$R" "464"

R=$(irc_send 2 "NICK testuser3\r\n")
assert_contains "NICK before PASS gets 464 error" "$R" "464"

R=$(irc_send 2 "PASS $PASSWORD\r\nJOIN #general\r\n")
assert_contains "Command before full registration gets 451" "$R" "451"

R=$(irc_send 2 "PASS\r\n")
assert_contains "Empty PASS gets 461 error" "$R" "461"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK\r\n")
assert_contains "Empty NICK gets 431 error" "$R" "431"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK toolongnick\r\n")
assert_contains "NICK over 9 chars gets 432 error" "$R" "432"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK 1invalid\r\n")
assert_contains "NICK starting with digit gets 432 error" "$R" "432"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK john@doe\r\n")
assert_contains "NICK with invalid char @ gets 432 error" "$R" "432"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK _valid1\r\nUSER _valid1 0 * :Test\r\n")
assert_contains "NICK with underscore is valid (001)" "$R" "001"

# R=$(irc_send 2 "PASS $PASSWORD\r\nNICK testuserA\r\nUSER testuserA\r\n")
# assert_contains "USER with less than 4 args gets 461" "$R" "461"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK testuserB\r\nUSER testuserB 0 * :Test\r\nUSER testuserB 0 * :Test\r\n")
assert_contains "Second USER (reregister) gets 462" "$R" "462"

# ─────────────────────────────────────────────────────────────────────────────
# print_section "2. NICK"
# # ─────────────────────────────────────────────────────────────────────────────

# # R=$(irc_send 2 "PASS $PASSWORD\r\nNICK nickchange1\r\nUSER nickchange1 0 * :Test\r\nNICK nickchange2\r\n")
# # assert_contains "Nick change after registration broadcasts NICK" "$R" "NICK nickchange2"

# # (printf "%b" "PASS $PASSWORD\r\nNICK dupnick\r\nUSER dupnick 0 * :Test\r\n"; sleep 4) | nc "$HOST" "$PORT" > /dev/null 2>&1 &
# # BG_PID=$!
# # sleep 1
# R=$(irc_send 2 "PASS $PASSWORD\r\nNICK dupnick\r\n")
# assert_contains "Duplicate nick gets 433 error" "$R" "433"
# kill $BG_PID 2>/dev/null; wait $BG_PID 2>/dev/null

# ─────────────────────────────────────────────────────────────────────────────
print_section "2. JOIN"
# ─────────────────────────────────────────────────────────────────────────────

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK joiner1\r\nUSER joiner1 0 * :Test\r\nJOIN #testjoin1\r\n")
assert_contains "JOIN new channel gets 353 names reply" "$R" "353"
assert_contains "JOIN new channel gets 366 end of names" "$R" "366"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK joiner2\r\nUSER joiner2 0 * :Test\r\nJOIN general\r\n")
assert_contains "JOIN without # prefix gets 476" "$R" "476"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK joiner3\r\nUSER joiner3 0 * :Test\r\nJOIN #\r\n")
assert_contains "JOIN with just # gets 476" "$R" "476"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK joiner4\r\nUSER joiner4 0 * :Test\r\nJOIN\r\n")
assert_contains "JOIN with no args gets 461" "$R" "461"

# (printf "%b" "PASS $PASSWORD\r\nNICK topicsetter\r\nUSER topicsetter 0 * :Test\r\nJOIN #topictest\r\nTOPIC #topictest :Hello World\r\n"; sleep 4) | nc "$HOST" "$PORT" > /dev/null 2>&1 &
# BG_PID=$!; sleep 1
# R=$(irc_send 2 "PASS $PASSWORD\r\nNICK joiner5\r\nUSER joiner5 0 * :Test\r\nJOIN #topictest\r\n")
# assert_contains "JOIN channel with topic gets 332" "$R" "332"
# kill $BG_PID 2>/dev/null; wait $BG_PID 2>/dev/null

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK joiner6\r\nUSER joiner6 0 * :Test\r\nJOIN #notopictest\r\n")
assert_contains "JOIN channel with no topic gets 331" "$R" "331"

# ─────────────────────────────────────────────────────────────────────────────
print_section "3. PART"
# ─────────────────────────────────────────────────────────────────────────────

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK parter1\r\nUSER parter1 0 * :Test\r\nJOIN #parttest\r\nPART #parttest\r\n")
assert_contains "PART channel you are in works" "$R" "PART"

# R=$(irc_send 2 "PASS $PASSWORD\r\nNICK parter2\r\nUSER parter2 0 * :Test\r\nPART #nonexistent\r\n")
# assert_contains "PART channel not joined gets 442" "$R" "442"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK parter3\r\nUSER parter3 0 * :Test\r\nPART #doesnotexist\r\n")
assert_contains "PART non-existent channel gets 403" "$R" "403"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK parter4\r\nUSER parter4 0 * :Test\r\nPART\r\n")
assert_contains "PART with no args gets 461" "$R" "461"

# ─────────────────────────────────────────────────────────────────────────────
# print_section "5. TOPIC"
# ─────────────────────────────────────────────────────────────────────────────

# R=$(irc_send 2 "PASS $PASSWORD\r\nNICK topicuser1\r\nUSER topicuser1 0 * :Test\r\nJOIN #topicview\r\nTOPIC #topicview\r\n")
# assert_contains "TOPIC view with no topic set gets 331" "$R" "331"

# R=$(irc_send 2 "PASS $PASSWORD\r\nNICK topicuser2\r\nUSER topicuser2 0 * :Test\r\nJOIN #topicset\r\nTOPIC #topicset :My Topic\r\n")
# assert_contains "TOPIC set broadcasts TOPIC message" "$R" "TOPIC"

# R=$(irc_send 2 "PASS $PASSWORD\r\nNICK topicuser3\r\nUSER topicuser3 0 * :Test\r\nJOIN #topicget\r\nTOPIC #topicget :GetMe\r\nTOPIC #topicget\r\n")
# assert_contains "TOPIC view after set gets 332" "$R" "332"

# R=$(irc_send 2 "PASS $PASSWORD\r\nNICK topicuser4\r\nUSER topicuser4 0 * :Test\r\nTOPIC #nosuchchan :test\r\n")
# assert_contains "TOPIC on non-existent channel gets 403" "$R" "403"

# (printf "%b" "PASS $PASSWORD\r\nNICK topicowner\r\nUSER topicowner 0 * :Test\r\nJOIN #topicnomember\r\n"; sleep 4) | nc "$HOST" "$PORT" > /dev/null 2>&1 &
# BG_PID=$!; sleep 1
# R=$(irc_send 2 "PASS $PASSWORD\r\nNICK topicuser5\r\nUSER topicuser5 0 * :Test\r\nTOPIC #topicnomember :test\r\n")
# assert_contains "TOPIC in channel you're not in gets 442" "$R" "442"
# kill $BG_PID 2>/dev/null; wait $BG_PID 2>/dev/null

# (printf "%b" "PASS $PASSWORD\r\nNICK topicop\r\nUSER topicop 0 * :Test\r\nJOIN #topicrestrict\r\nMODE #topicrestrict +t\r\n"; sleep 5) | nc "$HOST" "$PORT" > /dev/null 2>&1 &
# BG_PID=$!; sleep 1
# R=$(irc_send 2 "PASS $PASSWORD\r\nNICK topicnoob\r\nUSER topicnoob 0 * :Test\r\nJOIN #topicrestrict\r\nTOPIC #topicrestrict :hacked\r\n")
# assert_contains "TOPIC by non-op on +t channel gets 482" "$R" "482"
# kill $BG_PID 2>/dev/null; wait $BG_PID 2>/dev/null

# ─────────────────────────────────────────────────────────────────────────────
print_section "4. PRIVMSG & NOTICE"
# ─────────────────────────────────────────────────────────────────────────────

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK msger1\r\nUSER msger1 0 * :Test\r\nJOIN #msgtest\r\nPRIVMSG #msgtest :Hello!\r\n")
assert_not_contains "PRIVMSG to own channel has no error" "$R" "401|403|404|411"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK msger2\r\nUSER msger2 0 * :Test\r\nPRIVMSG #nonexistent :Hello!\r\n")
assert_contains "PRIVMSG to non-existent channel gets 403" "$R" "403"

# (printf "%b" "PASS $PASSWORD\r\nNICK msger3owner\r\nUSER msger3owner 0 * :Test\r\nJOIN #msgnomember\r\n"; sleep 4) | nc "$HOST" "$PORT" > /dev/null 2>&1 &
# BG_PID=$!; sleep 1
# R=$(irc_send 2 "PASS $PASSWORD\r\nNICK msger3\r\nUSER msger3 0 * :Test\r\nPRIVMSG #msgnomember :Hello!\r\n")
# assert_contains "PRIVMSG to channel not in gets 404" "$R" "404"
# kill $BG_PID 2>/dev/null; wait $BG_PID 2>/dev/null

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK msger4\r\nUSER msger4 0 * :Test\r\nPRIVMSG nobody :Hello!\r\n")
assert_contains "PRIVMSG to non-existent user gets 401" "$R" "401"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK msger5\r\nUSER msger5 0 * :Test\r\nPRIVMSG\r\n")
assert_contains "PRIVMSG with no args gets 411" "$R" "411"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK msger6\r\nUSER msger6 0 * :Test\r\nNOTICE nobody :Hello!\r\n")
assert_not_contains "NOTICE to non-existent user is silent (no 401)" "$R" "401"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK msger7\r\nUSER msger7 0 * :Test\r\nNOTICE\r\n")
assert_not_contains "NOTICE with no args is silent (no 411)" "$R" "411"

# ─────────────────────────────────────────────────────────────────────────────
print_section "5. KICK"
# ─────────────────────────────────────────────────────────────────────────────

# (printf "%b" "PASS $PASSWORD\r\nNICK kickvictim\r\nUSER kickvictim 0 * :Test\r\nJOIN #kicktest\r\n"; sleep 5) | nc "$HOST" "$PORT" > /dev/null 2>&1 &
# VICTIM_PID=$!; sleep 1
# R=$(irc_send 2 "PASS $PASSWORD\r\nNICK kickop\r\nUSER kickop 0 * :Test\r\nJOIN #kicktest\r\nKICK #kicktest kickvictim :bye\r\n")
# assert_contains "KICK as operator works" "$R" "KICK"
# kill $VICTIM_PID 2>/dev/null; wait $VICTIM_PID 2>/dev/null

(printf "%b" "PASS $PASSWORD\r\nNICK kickop2\r\nUSER kickop2 0 * :Test\r\nJOIN #kicktest2\r\n"; sleep 5) | nc "$HOST" "$PORT" > /dev/null 2>&1 &
OP_PID=$!; sleep 1
R=$(irc_send 2 "PASS $PASSWORD\r\nNICK kicknoob\r\nUSER kicknoob 0 * :Test\r\nJOIN #kicktest2\r\nKICK #kicktest2 kickop2 :bye\r\n")
assert_contains "KICK as non-operator gets 482" "$R" "482"
kill $OP_PID 2>/dev/null; wait $OP_PID 2>/dev/null

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK kickop3\r\nUSER kickop3 0 * :Test\r\nJOIN #kicktest3\r\nKICK #kicktest3 nobody :bye\r\n")
assert_contains "KICK non-existent user gets 441" "$R" "441"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK kickop4\r\nUSER kickop4 0 * :Test\r\nKICK #nonexistent someone :bye\r\n")
assert_contains "KICK on non-existent channel gets 403" "$R" "403"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK kickop5\r\nUSER kickop5 0 * :Test\r\nKICK\r\n")
assert_contains "KICK with no args gets 461" "$R" "461"

# ─────────────────────────────────────────────────────────────────────────────
print_section "6. INVITE"
# ─────────────────────────────────────────────────────────────────────────────

(printf "%b" "PASS $PASSWORD\r\nNICK invitee\r\nUSER invitee 0 * :Test\r\n"; sleep 5) | nc "$HOST" "$PORT" > /dev/null 2>&1 &
INVITEE_PID=$!; sleep 1
R=$(irc_send 2 "PASS $PASSWORD\r\nNICK inviteop\r\nUSER inviteop 0 * :Test\r\nJOIN #invitetest\r\nINVITE invitee #invitetest\r\n")
assert_contains "INVITE as operator gets 341 confirmation" "$R" "341"
kill $INVITEE_PID 2>/dev/null; wait $INVITEE_PID 2>/dev/null

# (printf "%b" "PASS $PASSWORD\r\nNICK inviteop2\r\nUSER inviteop2 0 * :Test\r\nJOIN #invitetest2\r\n"; sleep 5) | nc "$HOST" "$PORT" > /dev/null 2>&1 &
# OP2_PID=$!; sleep 1
# (printf "%b" "PASS $PASSWORD\r\nNICK invitee2\r\nUSER invitee2 0 * :Test\r\n"; sleep 5) | nc "$HOST" "$PORT" > /dev/null 2>&1 &
# INVITEE2_PID=$!; sleep 1
# R=$(irc_send 2 "PASS $PASSWORD\r\nNICK invitenoob\r\nUSER invitenoob 0 * :Test\r\nJOIN #invitetest2\r\nINVITE invitee2 #invitetest2\r\n")
# assert_contains "INVITE as non-operator gets 482" "$R" "482"
# kill $OP2_PID $INVITEE2_PID 2>/dev/null; wait $OP2_PID $INVITEE2_PID 2>/dev/null

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK inviteop3\r\nUSER inviteop3 0 * :Test\r\nJOIN #invitetest3\r\nINVITE nobody #invitetest3\r\n")
assert_contains "INVITE non-existent user gets 401" "$R" "401"

# (printf "%b" "PASS $PASSWORD\r\nNICK alreadyin\r\nUSER alreadyin 0 * :Test\r\nJOIN #invitetest4\r\n"; sleep 5) | nc "$HOST" "$PORT" > /dev/null 2>&1 &
# AIN_PID=$!; sleep 1
# R=$(irc_send 2 "PASS $PASSWORD\r\nNICK inviteop4\r\nUSER inviteop4 0 * :Test\r\nJOIN #invitetest4\r\nINVITE alreadyin #invitetest4\r\n")
# assert_contains "INVITE user already in channel gets 443" "$R" "443"
# kill $AIN_PID 2>/dev/null; wait $AIN_PID 2>/dev/null

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK inviteop5\r\nUSER inviteop5 0 * :Test\r\nINVITE someone #nonexistent\r\n")
assert_contains "INVITE to non-existent channel gets 403" "$R" "403"

# ─────────────────────────────────────────────────────────────────────────────
print_section "7. MODE"
# ─────────────────────────────────────────────────────────────────────────────

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK modeop1\r\nUSER modeop1 0 * :Test\r\nJOIN #modetest1\r\nMODE #modetest1\r\n")
assert_contains "MODE view returns 324" "$R" "324"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK modeop2\r\nUSER modeop2 0 * :Test\r\nJOIN #modetest2\r\nMODE #modetest2 +i\r\n")
assert_contains "MODE +i broadcasts mode change" "$R" "\+i"

(printf "%b" "PASS $PASSWORD\r\nNICK modeop3\r\nUSER modeop3 0 * :Test\r\nJOIN #inviteonly\r\nMODE #inviteonly +i\r\n"; sleep 5) | nc "$HOST" "$PORT" > /dev/null 2>&1 &
BG_PID=$!; sleep 1
R=$(irc_send 2 "PASS $PASSWORD\r\nNICK modenoob1\r\nUSER modenoob1 0 * :Test\r\nJOIN #inviteonly\r\n")
assert_contains "JOIN invite-only channel without invite gets 473" "$R" "473"
kill $BG_PID 2>/dev/null; wait $BG_PID 2>/dev/null

(printf "%b" "PASS $PASSWORD\r\nNICK modeop4\r\nUSER modeop4 0 * :Test\r\nJOIN #keytest\r\nMODE #keytest +k secret\r\n"; sleep 5) | nc "$HOST" "$PORT" > /dev/null 2>&1 &
BG_PID=$!; sleep 1
R=$(irc_send 2 "PASS $PASSWORD\r\nNICK modenoob2\r\nUSER modenoob2 0 * :Test\r\nJOIN #keytest wrongkey\r\n")
assert_contains "JOIN with wrong key gets 475" "$R" "475"
kill $BG_PID 2>/dev/null; wait $BG_PID 2>/dev/null

(printf "%b" "PASS $PASSWORD\r\nNICK modeop5\r\nUSER modeop5 0 * :Test\r\nJOIN #limittest\r\nMODE #limittest +l 1\r\n"; sleep 5) | nc "$HOST" "$PORT" > /dev/null 2>&1 &
BG_PID=$!; sleep 1
R=$(irc_send 2 "PASS $PASSWORD\r\nNICK modenoob3\r\nUSER modenoob3 0 * :Test\r\nJOIN #limittest\r\n")
assert_contains "JOIN full channel gets 471" "$R" "471"
kill $BG_PID 2>/dev/null; wait $BG_PID 2>/dev/null

# (printf "%b" "PASS $PASSWORD\r\nNICK futureop\r\nUSER futureop 0 * :Test\r\nJOIN #optest\r\n"; sleep 5) | nc "$HOST" "$PORT" > /dev/null 2>&1 &
# BG_PID=$!; sleep 1
# R=$(irc_send 2 "PASS $PASSWORD\r\nNICK modeop6\r\nUSER modeop6 0 * :Test\r\nJOIN #optest\r\nMODE #optest +o futureop\r\n")
# assert_contains "MODE +o grants operator" "$R" "\+o"
# kill $BG_PID 2>/dev/null; wait $BG_PID 2>/dev/null

# (printf "%b" "PASS $PASSWORD\r\nNICK modeop7\r\nUSER modeop7 0 * :Test\r\nJOIN #modenooptest\r\n"; sleep 5) | nc "$HOST" "$PORT" > /dev/null 2>&1 &
# BG_PID=$!; sleep 1
# R=$(irc_send 2 "PASS $PASSWORD\r\nNICK modenoob4\r\nUSER modenoob4 0 * :Test\r\nJOIN #modenooptest\r\nMODE #modenooptest +i\r\n")
# assert_contains "MODE by non-operator gets 482" "$R" "482"
# kill $BG_PID 2>/dev/null; wait $BG_PID 2>/dev/null

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK modeop8\r\nUSER modeop8 0 * :Test\r\nJOIN #unknownmode\r\nMODE #unknownmode +z\r\n")
assert_contains "Unknown mode char gets 472" "$R" "472"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK modeop9\r\nUSER modeop9 0 * :Test\r\nMODE #nonexistent +i\r\n")
assert_contains "MODE on non-existent channel gets 403" "$R" "403"

# ─────────────────────────────────────────────────────────────────────────────
print_section "8. PING"
# ─────────────────────────────────────────────────────────────────────────────

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK pinger1\r\nUSER pinger1 0 * :Test\r\nPING :12345\r\n")
assert_contains "PING gets PONG back" "$R" "PONG"
assert_contains "PONG echoes the token" "$R" "12345"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK pinger2\r\nUSER pinger2 0 * :Test\r\nPING\r\n")
assert_contains "PING with no token gets 409" "$R" "409"

# ─────────────────────────────────────────────────────────────────────────────
print_section "11. QUIT"
# ─────────────────────────────────────────────────────────────────────────────

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK quitter1\r\nUSER quitter1 0 * :Test\r\nQUIT :Goodbye\r\n")
assert_contains "QUIT with reason sends ERROR closing link" "$R" "ERROR"

R=$(irc_send 2 "PASS $PASSWORD\r\nNICK quitter2\r\nUSER quitter2 0 * :Test\r\nQUIT\r\n")
assert_contains "QUIT without reason sends ERROR" "$R" "ERROR"

# # ─────────────────────────────────────────────────────────────────────────────
# print_section "12. UNKNOWN COMMAND"
# # ─────────────────────────────────────────────────────────────────────────────

# R=$(irc_send 2 "PASS $PASSWORD\r\nNICK unknowncmd\r\nUSER unknowncmd 0 * :Test\r\nFOOBAR #test :hello\r\n")
# assert_contains "Unknown command gets 421" "$R" "421"

# ─────────────────────────────────────────────────────────────────────────────
# RESULTS
# ─────────────────────────────────────────────────────────────────────────────
echo ""
echo -e "${CYAN}${BOLD}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"
echo -e "${BOLD}  RESULTS${RESET}"
echo -e "${CYAN}${BOLD}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"
echo -e "  Total:  ${BOLD}$TOTAL${RESET}"
echo -e "  ${GREEN}Passed: $PASS_COUNT${RESET}"
echo -e "  ${RED}Failed: $FAIL_COUNT${RESET}"
echo ""
if [ $FAIL_COUNT -eq 0 ]; then
    echo -e "${GREEN}${BOLD}  All tests passed!${RESET}"
else
    echo -e "${RED}${BOLD}  $FAIL_COUNT test(s) failed.${RESET}"
fi
echo ""
