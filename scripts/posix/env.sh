


# this script should be sourced from the root of the repo. TODO see if there's a way to enforce it

# Add build/posix/bin to PATH
echo "Adding build/posix/bin to PATH"
PATH="$(realpath "build/posix/bin"):$PATH"
