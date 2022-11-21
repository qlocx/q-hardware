# exec vars are set in the calling script
# $1 = program name

printf "Executing $1"

(cd $1 && ./program.sh)