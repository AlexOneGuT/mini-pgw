#!/bin/bash
cmake -S . -B build && cmake --build build


LAUNCH_DIR="$(pwd)"

cat > "$LAUNCH_DIR/start" << 'EOF'
#!/bin/bash
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
"$SCRIPT_DIR/build/pgw_server"
EOF
chmod +x "$LAUNCH_DIR/start"