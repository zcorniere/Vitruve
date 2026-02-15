#!/usr/bin/env bash
set -e

echo "$@"

exe="$1"
shift

dest="$(dirname "$exe")"

echo "Copying runtime dependencies for: $exe"

copy_if_needed() {
    src="$1"
    dst="$dest/$(basename "$src")"

    # Skip if same file (same inode)
    if [ -e "$dst" ] && [ "$src" -ef "$dst" ]; then
        return
    fi

    cp -vu "$src" "$dest/"
}

# Copy linked shared libraries
ldd "$exe" | awk '
    /=>/ {
        if ($3 ~ /^\// && $3 !~ /^\/lib/ && $3 !~ /^\/usr\/lib/) print $3
    }
' | while read -r lib; do
    copy_if_needed "$lib"
done

# Copy explicitly provided plugin libs
for lib in "$@"; do
    copy_if_needed "$lib"
done
