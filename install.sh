#!/bin/bash
set -e

INSTALL_DIR="$HOME/.local/bin"
mkdir -p "$INSTALL_DIR"

echo "[1/3] Compiling library..."
g++ -std=c++17 -Wall -Wextra main.cpp db.cpp tableGen.cpp -o "$INSTALL_DIR/library" -lsqlite3

echo "[2/3] Checking PATH..."
if [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
    echo "export PATH=\"\$HOME/.local/bin:\$PATH\"" >> ~/.bashrc
    export PATH="$HOME/.local/bin:$PATH"
    echo "Added $INSTALL_DIR to ~/.bashrc"
fi

echo "[3/3] Initializing database..."
"$INSTALL_DIR/athena" init

echo "Installation complete! Run 'athena -init' in a new terminal."