git reset --hard HEAD
git pull

echo "You are starting to program board Q1. Press enter to continue or X to exit"

read -n 1 -s key

# if user input is X then exit
if [ "$key" = "X" ]; then
    exit 0
fi

# if user input is enter then continue
if [ "$key" = "" ]; then
    echo "Starting Q1 programming"
    
    (cd q1 && ./program.sh)
fi

exit 0
