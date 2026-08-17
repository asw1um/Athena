#!/bin/bash


if command -v library &> /dev/null; then
    DB_PATH="$HOME/.library.db"

    if [ ! -f "$DB_PATH" ]; then
        echo -e "\033[1;33m[INFO] First-time run detected. Initializing database...\033[0m"
        library init
    fi

    echo -e "\n\033[1;36m--- My Library ---\033[0m"
    library -ls
    echo ""
fi