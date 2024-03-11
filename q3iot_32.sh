git reset --hard HEAD
git pull


RED='\033[1;91m'
BLACK='\033[1;90m'

echo -e "${BLACK}"
echo "  _____                                               _               _                         _   ";
echo " |  __ \                                             (_)             | |                       | |_ ";
echo " | |__) | __ ___   __ _ _ __ __ _ _ __ ___  _ __ ___  _ _ __   __ _  | |__   ___   __ _ _ __ __| (_)";
echo " |  ___/ '__/ _ \ / _\` | '__/ _\` | '_ \` _ \| '_ \` _ \| | '_ \ / _\` | | '_ \ / _ \ / _\` | '__/ _\` |  ";
echo " | |   | | | (_) | (_| | | | (_| | | | | | | | | | | | | | | | (_| | | |_) | (_) | (_| | | | (_| |_ ";
echo " |_|   |_|  \___/ \__, |_|  \__,_|_| |_| |_|_| |_| |_|_|_| |_|\__, | |_.__/ \___/ \__,_|_|  \__,_(_)";
echo "                   __/ |                                       __/ |                                ";
echo "                  |___/                                       |___/                                 ";

echo ""
echo ""
echo ""
echo ""

echo -e "${WHITE_BG}${RED}"

echo "   ____    ____    _____   _______            _ _   _       ____ ___                     _       "
echo "  / __ \  |___ \  |_   _| |__   __|          (_) | | |     |___ \__ \                   | |      "
echo " | |  | |   __) |   | |  ___ | |    __      ___| |_| |__     __) | ) |  _ __   ___  _ __| |_ ___ "
echo " | |  | |  |__ <    | | / _ \| |    \ \ /\ / / | __| '_ \   |__ < / /  | '_ \ / _ \| '__| __/ __|"
echo " | |__| |  ___) |  _| || (_) | |     \ V  V /| | |_| | | |  ___) / /_  | |_) | (_) | |  | |_\__ |"
echo "  \___\_\ |____/  |_____\___/|_|      \_/\_/ |_|\__|_| |_| |____/____| | .__/ \___/|_|   \__|___/"
echo "                                                                       | |                       "
echo "                                                                       |_|                       "

echo -e "${BLACK}"
echo ""
echo ""
echo ""


echo "  _____                                _              _                          _   _                                __   __  _                    _ _   ";
echo " |  __ \                              | |            | |                        | | (_)                               \ \ / / | |                  (_) |  ";
echo " | |__) | __ ___  ___ ___    ___ _ __ | |_ ___ _ __  | |_ ___     ___ ___  _ __ | |_ _ _ __  _   _  ___    ___  _ __   \ V /  | |_ ___     _____  ___| |_ ";
echo " |  ___/ '__/ _ \/ __/ __|  / _ \ '_ \| __/ _ \ '__| | __/ _ \   / __/ _ \| '_ \| __| | '_ \| | | |/ _ \  / _ \| '__|   > <   | __/ _ \   / _ \ \/ / | __|";
echo " | |   | | |  __/\__ \__ \ |  __/ | | | ||  __/ |    | || (_) | | (_| (_) | | | | |_| | | | | |_| |  __/ | (_) | |     / . \  | || (_) | |  __/>  <| | |_ ";
echo " |_|   |_|  \___||___/___/  \___|_| |_|\__\___|_|     \__\___/   \___\___/|_| |_|\__|_|_| |_|\__,_|\___|  \___/|_|    /_/ \_\  \__\___/   \___/_/\_\_|\__|";
echo "                                                                                                                                                          ";
echo "                                                                                                                                                          ";


read -s key

# if user input is X then exit
if [ "$key" = "X" ]; then
    exit 0
fi

# if user input is enter then continue
if [ "$key" = "" ]; then
    echo "Starting Q3 32 programming"
    
    (cd Q3IoT-32 && ./program.sh)
fi

exit 0
