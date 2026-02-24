#!/bin/bash

FORMAT=$1
COUNT=$2

if [ -z "$FORMAT" ]; then
	printf "Usage: %s <format> [<count>]" "$0"
	exit 1
fi
if [ -z "$COUNT" ]; then
	COUNT=50
fi

LAST="$(date +%s)"
while [ $COUNT -gt 0 ]; do
	echo "[GET] https://replay.pokemonshowdown.com/search.json?format=$FORMAT&before=$LAST"
	BODY=$(curl --fail "https://replay.pokemonshowdown.com/search.json?format=$FORMAT&before=$LAST" 2>/dev/null) || exit
	VALUE=""
	INDEX=0
	while [ $COUNT -gt 0 ] && [ "$VALUE" "!=" null ]; do
		VALUE=$(echo "$BODY" | jq ".[$INDEX]")
		if [ "$VALUE" "!=" null ]; then
			ID="$(echo "$VALUE" | jq -r '.["id"]')"
			LAST="$(echo "$VALUE" | jq '.["uploadtime"]')"
			if ls "${ID}_${LAST}.log" >/dev/null 2>/dev/null; then
				LAST=$(ls | cut -f 2 -d '_' | cut -f 1 -d '.' | sort | head -n 1)
				VALUE=null
				echo "Skipping replays already downloaded..."
			else
				echo "[GET] https://replay.pokemonshowdown.com/$ID.log"
				curl --fail "https://replay.pokemonshowdown.com/$ID.log" --output "${ID}_${LAST}.log" 2>/dev/null || exit
				INDEX=$(($INDEX + 1))
				COUNT=$(($COUNT - 1))
			fi
		fi
	done
done
