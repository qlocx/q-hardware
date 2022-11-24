# git reset --hard HEAD
# git pull

RED='\033[1;91m'
BLACK='\033[1;90m'

# write with asci formatting

echo -e "${BLACK}"
echo "  _____                                               _               _                         _   ";
echo " |  __ \                                             (_)             | |                       | |_ ";
echo " | |__) | __ ___   __ _ _ __ __ _ _ __ ___  _ __ ___  _ _ __   __ _  | |__   ___   __ _ _ __ __| (_)";
echo " |  ___/ '__/ _ \ / _\` | '__/ _\` | '_ \` _ \| '_ \` _ \| | '_ \ / _\` | | '_ \ / _ \ / _\` | '__/ _\` |  ";
echo " | |   | | | (_) | (_| | | | (_| | | | | | | | | | | | | | | | (_| | | |_) | (_) | (_| | | | (_| |_ ";
echo " |_|   |_|  \___/ \__, |_|  \__,_|_| |_| |_|_| |_| |_|_|_| |_|\__, | |_.__/ \___/ \__,_|_|  \__,_(_)";
echo "                   __/ |                                       __/ |                                ";
echo "                  |___/                                       |___/                                 ";

echo -e ""
echo -e ""
echo -e ""
echo -e ""

echo -e "${WHITE_BG}${RED}"
echo "                                 ____        __ ";
echo "                                / __ \      /_ |";
echo "                               | |  | |      | |";
echo "                               | |  | |      | |";
echo "                               | |__| |      | |";
echo "                                \___\_\      |_|";


# transparend background

echo -e "${BLACK}"
echo -e ""
echo -e ""
echo -e ""

echo "  _____                                _              _                          _   _                                __   __  _                    _ _   ";
echo " |  __ \                              | |            | |                        | | (_)                               \ \ / / | |                  (_) |  ";
echo " | |__) | __ ___  ___ ___    ___ _ __ | |_ ___ _ __  | |_ ___     ___ ___  _ __ | |_ _ _ __  _   _  ___    ___  _ __   \ V /  | |_ ___     _____  ___| |_ ";
echo " |  ___/ '__/ _ \/ __/ __|  / _ \ '_ \| __/ _ \ '__| | __/ _ \   / __/ _ \| '_ \| __| | '_ \| | | |/ _ \  / _ \| '__|   > <   | __/ _ \   / _ \ \/ / | __|";
echo " | |   | | |  __/\__ \__ \ |  __/ | | | ||  __/ |    | || (_) | | (_| (_) | | | | |_| | | | | |_| |  __/ | (_) | |     / . \  | || (_) | |  __/>  <| | |_ ";
echo " |_|   |_|  \___||___/___/  \___|_| |_|\__\___|_|     \__\___/   \___\___/|_| |_|\__|_|_| |_|\__,_|\___|  \___/|_|    /_/ \_\  \__\___/   \___/_/\_\_|\__|";
echo "                                                                                                                                                          ";
echo "                                                                                                                                                          ";

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
