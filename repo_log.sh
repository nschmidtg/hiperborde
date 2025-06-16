# Set your base repository URL here (without the trailing slash)
BASE_URL="https://github.com/nschmidtg/hiperborde"
CSV_FILE="../${PWD##*/}.csv"

# Create a temp file to hold new lines
TEMP_FILE=$(mktemp)

for REMOTE_BRANCH in $(git branch -r | grep origin); do
    echo "$REMOTE_BRANCH"
    git log "$REMOTE_BRANCH" --format="%H|%ci|%an|%s" | \
    while IFS='|' read -r commit_hash date author subject; do
        commit_url="${BASE_URL}/commit/${commit_hash}"
        line="${date}|${author}|${subject}|${commit_url}"
        if ! grep -F -x "$line" "$CSV_FILE" > /dev/null; then
            echo "$line" >> "$TEMP_FILE"
        fi
    done
done

# Combine old CSV and new entries, sort by date (first column), remove duplicates, and save back
cat "$CSV_FILE" "$TEMP_FILE" | sort | uniq > "${CSV_FILE}.tmp"
mv "${CSV_FILE}.tmp" "$CSV_FILE"
rm "$TEMP_FILE"

