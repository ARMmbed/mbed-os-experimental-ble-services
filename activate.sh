ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# Test if the script is running on windows
windows() {
    [[ -n "$WINDIR" ]];
}

# Activate virtual environment
if windows; then
  source "$ROOT"/tests/TESTS/venv/Scripts/activate
else
  source "$ROOT"/tests/TESTS/venv/bin/activate
fi
